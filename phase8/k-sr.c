// k-sr.c, 159

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"
#include "tools.h"
#include "k-sr.h"
#include "proc.h"
#include "sys-call.h"

// to create a process: alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_q
void NewProcSR(func_p_t p) {  // arg: where process code starts
   int pid;
   int running_now;
   running_now = run_pid;

   if(QisEmpty(&pid_q)) {     // may occur if too many been created
      cons_printf("Panic: no more process!\n");
      breakpoint();
      
      run_pid = running_now;

      return;
  // cannot continue, alternative: breakpoint();
   }

   pid = DeQ(&pid_q);// alloc PID (1st is 0)
//printf("New pid = %d \n", pid);

   Bzero((char *)&pcb[pid], sizeof(pcb_t));                                       // clear PCB
   Bzero((char *)&proc_stack[pid][0], PROC_STACK_SIZE);                                // clear stack
   pcb[pid].state = READY;                                       // change process state

   if(pid > 0) EnQ(pid, &ready_q);                           // queue to ready_q if > 0

// point trapframe_p to stack & fill it out
   pcb[pid].trapframe_p =(trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE-sizeof(trapframe_t)];                // point to stack top
   //pcb[pid].trapframe_p--;                   // lower by trapframe size
   pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR; // enables intr
   pcb[pid].trapframe_p->cs = get_cs();                  // dupl from CPU
   pcb[pid].trapframe_p->eip =  (int)p;                         // set to code
}

void CheckWakeProc(void){
   int i;
   for(i = 0; i<sleep_q.tail; i++){
     int pid = DeQ(&sleep_q);
//     printf("Checking %d\n", pid);
     if(pcb[pid].wake_centi_sec ==sys_centi_sec) {
       EnQ(pid, &ready_q);
       pcb[pid].state = READY;
     }
     else EnQ(pid, &sleep_q);
   }
}

// count run_count and switch if hitting time slice
void TimerSR(void) {
   outportb(PIC_CONTROL, TIMER_DONE);                              // notify PIC timer done
  sys_centi_sec++;
 // printf("CheckingWakeProc now\n");
  CheckWakeProc();
  if(run_pid==0){
       return;
  }
   pcb[run_pid].run_count++;                                       // count up run_count
   pcb[run_pid].total_count++;                                        // count up total_count

   if(pcb[run_pid].run_count==TIME_SLICE) {       // if runs long enough
      EnQ(run_pid, &ready_q);                                    // move it to ready_q
      pcb[run_pid].state = READY;                                   // change its state
      run_pid = NONE;                                    // running proc = NONE
  } 
   
}

int GetPidSR(void){
  return run_pid;
}

void SleepSR(int centi_sec){
  
  pcb[run_pid].wake_centi_sec = (sys_centi_sec + centi_sec);
  pcb[run_pid].state = SLEEP;
  EnQ(run_pid, &sleep_q);
  run_pid=NONE;
}

void ShowCharSR(int row, int col, char ch){
  unsigned short *p = VID_HOME;
  p+=row*80 + col;
  *p = ch + VID_MASK;
//  cons_printf("%c\n", ch);


  }


//Phase 3
int MuxCreateSR(int flag){
  int newMux;
  newMux = DeQ(&mux_q);
  //printf("Created Mux with flag %d\n", flag);
  mux[newMux].flag = flag;
  mux[newMux].creater = run_pid;
  Bzero((char *)&(mux[newMux].suspend_q), sizeof(q_t));

  return newMux;
  
}

void MuxOpSR(int mux_id, int opcode){
   if(opcode==LOCK){
	//printf("Attempting Lock, flag = %d\n", mux[mux_id].flag);
      if(mux[mux_id].flag>0) {
      mux[mux_id].flag--;
      //printf("Mux %d locked, value is %d\n", mux_id, mux[mux_id].flag);
      
      }
      else{
          EnQ(run_pid, &(mux[mux_id].suspend_q));
          pcb[run_pid].state = SUSPEND;
          run_pid = NONE;
          //printf("Mux %d already locked, item queued\n", mux_id);
          

        }
      //printf("Mux: %d locked\n",mux_id); 
     }
     
   else if(opcode == UNLOCK){
     if(QisEmpty(&(mux[mux_id].suspend_q))) {
       
       mux[mux_id].flag++;
       //printf("Mux: %d unlocked, value is %d\n", mux_id, mux[mux_id].flag);

     }
     else{
        int newRunningID;
        newRunningID = DeQ(&(mux[mux_id].suspend_q));
        EnQ(newRunningID, &ready_q);
        pcb[newRunningID].state = READY;
        //printf("Mux: %d unlocked, PID: newRunningID : %d ready\n", mux_id, newRunningID);
       }
     }
}

// JR: FOR PHASE 4

void TermSR(int term_no) {
        int x = inportb(term[term_no].io_base+IIR);
       
      //printf("Start of TermSR term_no = %d; IIR = %d\n", term_no, x);
  //    printf("IIR options TXRDY = %d, RXRDY = %d\n", TXRDY, RXRDY);
      //read the type of event from IIR of the terminal port
      //(IIR is Interrupt Indicator Register)
	      if(x == TXRDY){
          //printf("Moving into TermTxSR\n");
          	TermTxSR(term_no);
		 } 
     
     else if (x == RXRDY) {
       //printf("Moving into TermRxSR to return\n");
    TermRxSR(term_no);
	  } 

    //printf("Out of Term**SR\n");
	  if (term[term_no].tx_missed == TRUE) { //the tx_missed flag is TRUE, also call TermTxSR(term_no)
		TermTxSR(term_no);
	  }

} 

   void TermTxSR(int term_no) {
    char c; 
    //printf("TermTxSR term_no = %d\n", term_no);
    //if the out_q in terminal interface data structure is empty:
         //if (QisEmpty(&(term[term_no]))) {
//         printf("Checking if out_q is empty\n");
         if (QisEmpty(&term[term_no].out_q) && QisEmpty(&term[term_no].echo_q)) {
             //printf("Queue is empty\n");
           term[term_no].tx_missed = TRUE;
		return;							//2. return
	}
 
     else {
		if(!QisEmpty(&term[term_no].echo_q)){
              		c = DeQ(&term[term_no].echo_q);
		}
		else{
			c = DeQ(&term[term_no].out_q);
			MuxOpSR(term[term_no].out_mux, UNLOCK);
		}
		outportb(term[term_no].io_base+DATA,c);
		term[term_no].tx_missed = FALSE;
              
         } 
          //printf("Dequeued %c, sending to DATA\n", c);
          //outportb(term[term_no].io_base+DATA,c);//2. use outportb() to send it to the DATA register of the terminal port 
			    //printf("Sent %c to DATA\n", c);
//          printf("term_no is %d\n", term_no);
          //term[term_no].tx_missed = FALSE;//3. set the tx_missed flag to FALSE
    			//printf("term_no before unlock is %d\n", term_no);
          //MuxOpSR(term[term_no].out_mux, UNLOCK); //4. unlock the out_mux of the terminal interface data structure
	  
    //UNLOCK TERM_ID incorrect and doesnt work
    
//    printf("Unlocked Mux id %d\n", term[term_no].out_mux);
     

   }

   void TermRxSR(term_no) {
     char ch = inportb(term[term_no].io_base+DATA);
     int ctrlc = 3;
     EnQ(ch, &(term[term_no].echo_q));

	//Phase 7
		
	if(ch == (char)ctrlc){
		cons_printf("CtrlC detected\n");
		if(!QisEmpty(&mux[term[term_no].in_mux].suspend_q)){
			if(pcb[mux[term[term_no].in_mux].suspend_q.q[0]].sigint_handler!=NONE){
				cons_printf("Calling WrapperSR\n");
				WrapperSR(mux[term[term_no].in_mux].suspend_q.q[0], pcb[mux[term[term_no].in_mux].suspend_q.q[0]].sigint_handler, term_no);
			}
		}
		
	}


     if(ch== '\r') {
       EnQ('\n',&(term[term_no].echo_q));
       
     }
     if(ch == '\r') { // for some reason this is its own sperate if from coding hints
       EnQ('\0', &(term[term_no].in_q));  
     } 
	else {
          EnQ(ch, &(term[term_no].in_q));
       }

       MuxOpSR(term[term_no].in_mux, UNLOCK);
	  // return;
	   
   }

int ForkSR(void){
		
	int ppid = run_pid;
	int cpid;
	int *p;
	int stackDif;
	//cons_printf("Starting ForkSR\n");
	if (QisEmpty(&pid_q)){
		cons_printf("Panic: PID Queue is Empty\n");
		return NONE;	

	}

	cpid = DeQ(&pid_q);
	//cons_printf("CPID = %d\n",cpid); 
	Bzero((char *)&pcb[cpid], sizeof(pcb_t));

	pcb[cpid].state = READY;
	pcb[cpid].ppid = ppid;

	EnQ(cpid, &ready_q);
	
	stackDif= &proc_stack[cpid][0] - &proc_stack[ppid][0];
	//cons_printf("Diff = %d\n", stackDif);	
	pcb[cpid].trapframe_p = (trapframe_t *)((int)pcb[ppid].trapframe_p+stackDif);
	//cons_printf("ppid trapframe = %d, cpid trapframe = %d\n", pcb[ppid].trapframe_p, pcb[cpid].trapframe_p);
	MemCpy((char *)&proc_stack[ppid][0]+stackDif, (char *)&proc_stack[ppid][0], PROC_STACK_SIZE);

	pcb[cpid].trapframe_p->eax = 0;
		
	

	
	//Adjust location adjustment esp, ebp, esi, edi
	pcb[cpid].trapframe_p->esp =pcb[cpid].trapframe_p->esp+stackDif;
	pcb[cpid].trapframe_p->ebp =pcb[cpid].trapframe_p->ebp+stackDif;
	pcb[cpid].trapframe_p->esi =pcb[cpid].trapframe_p->esi+stackDif;
	pcb[cpid].trapframe_p->edi =pcb[cpid].trapframe_p->edi+stackDif;
	//cons_printf("ppid ebp = %d, cpid ebp = %d\n", pcb[ppid].trapframe_p->ebp,pcb[cpid].trapframe_p->ebp);

	p = (int *)pcb[cpid].trapframe_p ->ebp;
	//cons_printf("Initial *p = %d\n", *p);

	while(*p!=0){
					
		*p = *p + stackDif;
		
		p = (int *)*p;
		//cons_printf("New *p = %d\n", *p);
	}

	//cons_printf("Fork returning cpid: %d\n", cpid);	
	return cpid;
}	

int WaitSR(void){
	int i, j, code;
	
	for(i=0; i<PROC_SIZE; i++){
		if((run_pid == pcb[i].ppid) && (pcb[i].state == ZOMBIE)){
			break;
		}
	}
	//cons_printf("i: %d	PROCSIZE: %d\n", i, PROC_SIZE);
	
	if(i==PROC_SIZE){
		pcb[run_pid].state = WAIT;
		run_pid = NONE;
		return NONE;
	}
	
	code = pcb[i].trapframe_p->eax;
	//cons_printf("code: %d\n", code);

	pcb[i].state = UNUSED;
	EnQ(i, &pid_q);

	//Phase 7
	for(j = 0; j<PAGE_SIZE;j++){
		if(page_user[j]==i){
			page_user[j] = NONE;
		}

	}
	return code;

} 

void ExitSR(int exit_code){
	//cons_printf("Exitting with code: %d\n", exit_code);
	int i;
	int ppid = pcb[run_pid].ppid;
	if(pcb[ppid].state!=WAIT){
	//	cons_printf("Parent not waiting\n");
		pcb[run_pid].state = ZOMBIE;
		run_pid = NONE;
		return;
	}
	//cons_printf("Parent is waiting\n");
	pcb[ppid].state = READY;
	EnQ(ppid, &ready_q);
	pcb[ppid].trapframe_p->eax = exit_code;
	for(i = 0; i<PAGE_NUM; i++){
		if(page_user[i] == run_pid){
			page_user[run_pid] = NONE;
		}
	}
	pcb[run_pid].state = UNUSED;
	EnQ(run_pid, &pid_q);
	run_pid = NONE;
}
	//Phase 7 FOX
void ExecSR(int code, int arg){
	int * codeAddress;
	int * stackAddress;
	int i;
	(int)codeAddress = NONE;
	(int)stackAddress =NONE;
	for(i = 0; i<PAGE_NUM; i++){
		//cons_printf("page_user[%d] = %d\n", i, page_user[i]);
		if(page_user[i]==NONE){
		//	cons_printf("Selected page_user[%d]\n", i);	
			if ((int)codeAddress==NONE){
				page_user[i] = run_pid;	
				codeAddress = (int *)((i* PAGE_SIZE)+RAM);
			}
			else if((int)stackAddress==NONE){
				page_user[i] = run_pid;
				stackAddress = (int *)((i*PAGE_SIZE)+RAM);
			}
		}
		if(((int)codeAddress!=NONE)&&((int)stackAddress!=NONE)){
			break;
		}		
	}

	//cons_printf("Run_pid = %d\n", run_pid);
	//cons_printf("Code Segment Address = %x\n", codeAddress);
	//cons_printf("Stack Segment Address = %x\nAttemting code copy\n", stackAddress);
	//cons_printf("Code Address = %x\n", code);
	//Code Operations
	//
	//Bzero((char *)codeAddress, PAGE_SIZE);	
	MemCpy((char *)codeAddress, (char *)code, PAGE_SIZE);	
		
	//cons_printf("Clearing stack\n");
	//Stack Operations
	Bzero((char *)stackAddress, PAGE_SIZE);
	stackAddress = (int*)((int)stackAddress+PAGE_SIZE);
	stackAddress--;

	//cons_printf("Setting stackAddress to arg\n");
	*stackAddress = arg;
	//cons_printf("Address %x = arg=%d\n", stackAddress, arg);
	//cons_printf("Moving stackAddress by 4\n");
	stackAddress--;
	//cons_printf("Address %x\n", stackAddress);
	//cons_printf("Setting trapframe to stackAddress\n");
	
	pcb[run_pid].trapframe_p = (trapframe_t *)stackAddress;
	//(int *)pcb[run_pid].trapframe_p-=2;
	//cons_printf("Decrementing trapframe_p by 1\n");
	//cons_printf("Temp TPAddress before --%x\n", pcb[run_pid].trapframe_p);
	pcb[run_pid].trapframe_p--;
	//cons_printf("Temp TPAddress %x\n", pcb[run_pid].trapframe_p);
	
	//cons_printf("Getting efl\n");
	pcb[run_pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR; // enables intr
   	//cons_printf("Getting cs\n");
	pcb[run_pid].trapframe_p->cs = get_cs();                  // dupl from CPU
   	//cons_printf("Setting eip\n");
	pcb[run_pid].trapframe_p->eip =  (int)codeAddress;
	//cons_printf("Trapframe EIP: %x\n", pcb[run_pid].trapframe_p->eip);
	//cons_printf("End of Code\n");
	
	//pcb[run_pid].trapframe_p = (trapframe_t *)tempTP;
}

void SignalSR(int sig_num, int handler){
	pcb[run_pid].sigint_handler = handler;
}

void WrapperSR(int pid, int handler, int arg){
	int *tempTP;
	tempTP = (int *)pcb[run_pid].trapframe_p;
	tempTP = (int *)((int)tempTP + sizeof(trapframe_t));
	MemCpy((char *)((int)pcb[pid].trapframe_p-sizeof(int[3])), (char *)pcb[run_pid].trapframe_p, sizeof(trapframe_t));
	pcb[pid].trapframe_p = (trapframe_t *)(pcb[run_pid].trapframe_p - sizeof(int[3])); 
	*tempTP = arg;
	tempTP--;
	*tempTP = handler;
	tempTP--;
	*tempTP = pcb[run_pid].trapframe_p->eip;
	pcb[run_pid].trapframe_p->eip = (int)Wrapper;

}

void PauseSR(void) {
  pcb[run_pid].state = WAIT;
  run_pid = NONE;
}

void KillSR(int pid, int sig_num) {
  if (pid == 0) {
          
    }  
}

unsigned RandSR(void) {
  int rand = run_pid * rand + A_PRIME;
  rand %= G2;
  return rand;
}
