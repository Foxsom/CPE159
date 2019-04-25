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
   int my_pid = GetPidCall();
   int forkPid, exitCode;
   char childPIDPrint[] = "Child PID:     ";
   char exitCodeStr[] = "Exit Code:    ";  
   //char str1[STR_SIZE] = "PID    process is running exclusively using the video display...";
   //char str2[STR_SIZE] = "                                                                ";
   char str1[STR_SIZE] = "PID    > ";
   char str2[STR_SIZE];

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
   	
	if(!StrCmp(str2, "fork")){
		continue;
	}
	
	forkPid = ForkCall();
	
	//cons_printf("Done with Fork\n");
	if(forkPid==NONE){
		WriteCall(which_term, "Couldn't fork!");
		continue;
	}

	if(forkPid==0){
		//cons_printf("Child pid: Moving to Aout\n");
		
		//Phase 7 FOX
		cons_printf("Entering ExecCall\n");
		//Aout(which_term);
		ExecCall((int)Aout, which_term);
		
		//Aout(which_term);
	}
	//cons_printf("User forkPID = %d\n", forkPid);
	//childPIDPrint[] = "Child PID:   ";
	childPIDPrint[11] = (forkPid/10)+'0';
	childPIDPrint[12] = (forkPid%10)+'0';
	WriteCall(which_term, childPIDPrint);
	WriteCall(which_term, "\n\r");
	//cons_printf("%c\n", childPIDPrint);
	exitCode = WaitCall();
	Itoa((char *)&exitCodeStr[11], exitCode);
	//cons_printf("%d\n", exitCode);
	WriteCall(which_term, exitCodeStr);
	WriteCall(which_term, "\n\r");
    }
}

void Aout(int device){
/*	
	int pid = GetPidCall();
	char str[] = "xx ( ) HelloWorld!\n\r";
	char alph = '@'+ pid;
	int exitCode = pid*100;
	int column = 0;	
	
	cons_printf("Starting Aout\n");
	

	str[0] = (pid/10)+'0';
	str[1] = (pid%10)+'0';	
	str[4] = alph;
	
	WriteCall(device, str);

	for(column=0; column<70; column++){
		ShowCharCall(pid, column, alph);
		SleepCall(10);
		ShowCharCall(pid, column, ' ');
	}	

	ExitCall(exitCode);	
*/

int column;
int pid = GetPidCall();
int exitCode = pid*100;
char str[] = "xx ( ) Hello World!\n\r";

//CHANGE STR
WriteCall(device, str);
for (column = 0; column<70; column++){
  ShowCharCall(pid, column, pid+'@');
  SleepCall(10);
  ShowCharCall(pid, column, ' ');
  }

ExitCall(exitCode);
}
//Phase 7 FOX
void Ouch(int device){
	WriteCall(device, "Can't touch that!\n\n");
}

void Wrapper(int handler, int arg){
	func_p_t2 func = (func_p_t2)handler;
	
	asm("pushal");
	func(arg);
	asm("popal");
	asm("movl %%ebp, %%esp;
		popl %%ebp;
		ret $8"::);

}
