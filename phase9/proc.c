// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use sys-calls

#include "k-const.h"   // LOOP
#include "sys-call.h"  // all service calls used below
#include "k-data.h"
#include "tools.h"
#include "k-include.h"
#include "proc.h"

void InitTerm(int term_no){
  int i, j;

  Bzero((char *)&term[term_no].out_q, sizeof(q_t));
  Bzero((char *)&term[term_no].in_q, sizeof(q_t));
  Bzero((char *)&term[term_no].echo_q, sizeof(q_t));

  //printf("Creating term Mux\n");
  term[term_no].out_mux = MuxCreateCall(Q_SIZE);
  term[term_no].in_mux = MuxCreateCall(0);
  
//printf("Term_no %d out_mux = %d, in_mux = %d\n", term_no, term[term_no].out_mux, term[term_no].in_mux);

  outportb(term[term_no].io_base+CFCR, CFCR_DLAB);
  outportb(term[term_no].io_base+BAUDLO, LOBYTE(115200/9600));
  outportb(term[term_no].io_base+BAUDHI, HIBYTE(115200/9600));
  outportb(term[term_no].io_base+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);

  outportb(term[term_no].io_base+IER, 0);
  outportb(term[term_no].io_base+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
  //printf("outports completed\n");
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
  //printf("First loop done\n");
  outportb(term[term_no].io_base+IER, IER_ERXRDY|IER_ETXRDY);
  //outportb(term[term_no].io_base+IER, IER_ETXRDY);
  //printf("TXRDY and RXRDY sent\n");
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
 
  //printf("Clearing screen\n");
  for(j=0; j<25; j++){
    outportb(term[term_no].io_base+DATA, '\n');
    for(i=0; i<LOOP/30; i++) asm("inb $0x80");
    outportb(term[term_no].io_base+DATA, '\r');
    for(i=0; i<LOOP/30; i++) asm("inb $0x80");
  }
  //printf("Running first inport\n");
  inportb(term[term_no].io_base);
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
}

void InitProc(void) {
   int i;
   //printf("Creating vid mux\n");
   vid_mux = MuxCreateCall(1);
   
   

   InitTerm(0);
   InitTerm(1);
   printf("Done initializing\n");
   while(1) {
      ShowCharCall(0, 0, '.');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service

      ShowCharCall(0, 0, ' ');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service

   }
}

void UserProc(void) {
//   int which_term;
   
   int i;
   int my_pid = GetPidCall();
   int forkPid;
   int exitCode;
   char childPIDPrint[] = "Child PID:     ";
   //char exitCodeStr[] = "Exit Code: ";
   //char exitCodeStrConv[STR_SIZE];  
   //char str1[STR_SIZE] = "PID    process is running exclusively using the video display...";
   //char str2[STR_SIZE] = "                                                                ";
   char str1[STR_SIZE] = "PID    > ";
   char str2[STR_SIZE];
   char arriveStr1[STR_SIZE] = "Child exit code: ";
   char arriveStr2[STR_SIZE] = "  arrives!";
   //char test[];
   //char test;
   int which_term = my_pid%2==1? TERM0_INTR : TERM1_INTR;
   //printf("PID %d interrupt is %d\n", my_pid, which_term);
  // if(my_pid%2==1) which_term = TERM0_INTR;
  // else which_term = TERM1_INTR;
  //func_p_t2 *OuchPtr = Ouch(which_term);
  //func_p_t2 *AoutPtr = Aout(which_term);
   str1[4] = '0' + my_pid/10;
   str1[5] = '0' + my_pid%10;
   
   //Phase 7 FOX
   SignalCall(SIGINT, (int)Ouch);

   while(1) {

      //MuxOpCall(vid_mux, LOCK);
      
      /*
//      printf("writing to STDOUT %d\n", STDOUT);
      WriteCall(STDOUT, str1);  // show my PID
 //     printf("wrinting to term %d\n", which_term);
      WriteCall(which_term, str1);
      WriteCall(which_term, "\n\r");
      SleepCall(50);

                     // erasure
 //     printf("Erasing text from display\n");
      WriteCall(STDOUT, str2);
      SleepCall(50);
      MuxOpCall(vid_mux, UNLOCK);
	*/
	WriteCall(which_term, str1);
	ReadCall(which_term, str2);
	//WriteCall(STDOUT, str2);
   	
	if(!StrCmp(str2, "race\0")){
		continue;
	}

	for(i = 0; i<5; i++){
		forkPid = ForkCall();
	
		//cons_printf("Done with Fork\n");
		if(forkPid==NONE){
			WriteCall(which_term, "Couldn't fork!\0");
			continue;
		}

		if(forkPid==0){
		
			ExecCall((int)Aout, which_term);	
		}
		//childPIDPrint[11] = (forkPid/10)+'0';
		//childPIDPrint[12] = (forkPid%10)+'0';
		Itoa(&childPIDPrint[11],forkPid);
    Itoa(&childPIDPrint[12],forkPid);
    WriteCall(which_term, childPIDPrint);
		WriteCall(which_term, "\n\r");
	}


	SleepCall(300);
	KillCall(0, SIGGO);
	
	
	for(i = 0; i<5; i++){
    //KillCall(0, SIGGO);
		exitCode = WaitCall();
		Itoa(&arriveStr1[11], exitCode);
		WriteCall(which_term, arriveStr1);
    WriteCall(which_term, " ");
    arriveStr2[0] = exitCode/100+'A';
		WriteCall(which_term, arriveStr2);
		WriteCall(which_term, "\n\r");
		//WriteCall(which_term, arriveStr1);
    //WriteCall(which_term, exitCodeStrConv);
		//WriteCall(which_term, arriveStr2);	
    	}
	
	}
}

void Aout(int device){	

int column=0;
int pid = GetPidCall();
int rando;

int exitCode = pid*100;
char str[] = "   ( ) Hello World!\n\r";
//char pidStr[STR_SIZE];
//char letter[STR_SIZE];
//letter[0] = '@'+pid;;

str[0] = '0'+pid/10;
str[1] = '0'+pid%10;

str[4] = 'A'+pid;
//Itoa(pidStr, pid);
//WriteCall(device, pidStr);
//WriteCall(device,str);
//WriteCall(device, " (");
//WriteCall(device, letter);

//CHANGE STR

WriteCall(device, str);
PauseCall();
//cons_printf("Unpaused: %d\n", run_pid);
for (column = 0; column<70; column++){
  ShowCharCall(pid, column, str[4]);
  rando = RandCall() %20+5;
  //SleepCall(RandCall());
  SleepCall(rando);
  ShowCharCall(pid, column, ' ');
  }

ExitCall(exitCode);
}

//Phase 7 FOX
void Ouch(int device){
	WriteCall(device, "Can't touch that!\n\r");
}

void Wrapper(int handler, int arg){
	func_p_t2 func = (func_p_t2)handler;
	
	asm("pushal");
	func(arg);
	asm("popal");
	asm("movl %%ebp, %%esp;
		popl %%ebp;
		ret $8"
    :
    :
  );

}
