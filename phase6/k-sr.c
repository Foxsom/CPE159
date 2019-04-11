// k-sr.c, 159 test

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"
#include "k-lib.h"
#include "k-sr.h"
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
     
     EnQ(ch, &(term[term_no].echo_q));

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

// for phase 6 JR
int ForkSR(void) {
    printf("ForkSR\n");
	/*
    int parentPID = GetPIDSR();
    int cpid; // childs pid
    int *p;
    //a. if pid_q is empty:
    if(QisEmpty(&pid_q)) {
          //1. prompt panic msg to PC
          cons_printf("Panic: PID Queue is Empty");
          //return NONE
          return NONE;
      }
    
    //b. get a child PID from pid_q (build a new PCB for new child process)
    cpid = DeQ(&pid_q);

    //c. clear the child PCB
    Bzero((char *)&pcb[cpid], sizeof(pcb_t));    

    //d. set the state in the child PCB to ...
    pcb[cpid].state = READY;   

    //e. set the ppid in the child PCB to ...
    pcb[cpid].ppid = parentPID;    

    //f. enqueue the child PID to ...
    EnQ(cpid, &ready_q);

    //g. get the differnce between the locations of teh child's stack and the parent's 
    int stackDif = &proc_stack[cpid] - &proc_stack[parentPID];

    //h. copy the parent's trapframe_p  to the childs trapframe_p
    pcb[cpid].*trapframe_p = pcb[ppid].*trapframe_p;

    //i. copy the parent's proc stack to the child (use your own MemCpy())
    MemCpy(&proc_stack[cpid], &proc_stack[ppid], PROC_STACK_SIZE);

    //j.set the eax in the new trapframe to 0 (child proc gets 0 from ForkCall())
   pcb[cpid].trapframe_p -> eax = 0;

    //k. apply the location adjustment to esp, ebp, esi, edi, in the new trapframe 
    //(nested base ptrs adjustment:)
    //pcb[cpid].trapframe_p -> esp = ;
    //pcb[cpid].trapframe_p -> ebp = ;
    //pcb[cpid].trapframe_p -> esi = ;
    //pcb[cpid].trapframe_p -> edi = ;

    //l. set an integer pointer p to ebp in the new trapframe
    p = &pcb[cpid].trapframe_p -> ebp;       
    //m. while (what p points to is not 0) 
    while (*p != 0){
       // 1. apply the location adjustment to the value pointed 
       // 2. set p to the adjusted value (need a typecast)
      }
    
    //n. return child PID
    return cpid;
*/ return 0;
}

int WaitSR(void) { //parent waits
    printf("WaitSR\n");
    /*
    // loop thru the PCB array (looking for a ZOMBIE child):
    // the proc ppid is run_pid and the state is ZOMBIE -> break the loop
   int i;
   int cpid; //NEEDS TO BE SET
   int code;
   for(i = 0; i< PROC_SIZE; i++) {
          if(run_pid == pcb[run_pid].ppid && pcb[i].state = ZOMBIE) {
              break;
            }
            
      }    
    //if the whole PCB array was looped thru (didnt find any ZOMBIE child):
    if(i==PROC_SIZE){
      	//1. alter the state of run_pid to ...
      	pcb[run_pid].state = WAIT;
      	//2. set run_pid to ...
      	run_pid = NONE;
      	//3. return NONE
      	return NONE;
      }

    // get its exit code (from the eax of the child's trapframe)
      code = pcb[cpid].trapframe_p->eax;
    // reclaim child's recoruses:
      //1. alter its state to ..
      pcb[cpid].state = UNUSED;
      //2. enqueue its PID to ...
      EnQ(cpid, pid_q);
    // return the exit code 
    return code;
*/
return 1;
}

void ExitSR(int exit_code) {
     printf("ExitSR\n");
     /*
    //if the process state of my parent (ppid) is not WAIT:
    int ppid = pcb[run_pid].ppid;
    if(pcb[ppid].state != WAIT){
      //1. alter my state to ...
      pcb[run_pid].state = ZOMBIE;
      //2. reset run_pid to ...
      run_pid = NONE;
      //3. return
      return;
    }

    // alter the state of ppid to ...
      pcb[ppid].state = READY;
    // enqueue ppid to ...
      EnQ(ppid, ready_q);
    // dont forget to pass it the exit code (via eax of parents trapframe)
      pcb[ppid].trapframe->eax = exit_code;

    //reclaim childs resources:
      //1.alter its state to ...
 	pcb[run_pid].state = UNUSED;	
     //2. enqueue its PID to ...
        EnQ(run_pid, pid_q);
      //reset run_pid to ...
        run_pid = NONE;

*/
}

