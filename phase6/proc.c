// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use sys-calls

#include "k-const.h"   // LOOP
#include "sys-call.h"  // all service calls used below
#include "k-data.h"
#include "k-lib.h"
#include "k-include.h"

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
   //char str1[STR_SIZE] = "PID    process is running exclusively using the video display...";
   //char str2[STR_SIZE] = "                                                                ";
   char str1[STR_SIZE] = "PID    > ";
   char str2[STR_SIZE];

   int which_term = my_pid%2==1? TERM0_INTR : TERM1_INTR;
   //printf("PID %d interrupt is %d\n", my_pid, which_term);
  // if(my_pid%2==1) which_term = TERM0_INTR;
  // else which_term = TERM1_INTR;
   str1[4] = '0' + my_pid/10;
   str1[5] = '0' + my_pid%10;
   
   while(1){
	
	WriteCall(which_term, str1); // promt for terminal input
	ReadCall(which_term, str2); // read terminal input
  
  // compare str2 and "fork", if not the same -> "continue;"
  
  // make a ForkCall() and get its return
  
  //if it returns NONE {
      //promt to terminal: "Couldn't fork!"
      //continue;
  //}
  
  //if the return is == 0 --> call Aout(device); child code
  
  //promt to terminal: the child PID //parent code
  
  //additional prompt to terminal "\n\r" would look better like the demo
  
  // make a WaitCall() and get an exit code from child
  
  //prompt to terminal: the child exit code (see demo)
  
  //must try your Itoa() (in k-lib.c) to convert exit code to str for prompting

  //additional prompt to terminal "\n\r" would look better like the demo

	//WriteCall(STDOUT, str2); MAY NOT NEED THIS- FROM CODING HINTS
   }
}


void Aout(int device) {
    //get my PID
    char str[] = "xx ( ) HelloWorld!\n\r";

    //in the above str, replace xx with my PID, and a alpabet inside
    //the bracket (alphabet B if my PID is 2, C if 3, D if 4, ect)
    
    //prompt to terminal the str // use same device as parent
    
    //slide my alphabet across the Target PC display:
    //cycle thru columns 0 to 69 {
        //use ShowCharCall( ...
        //use Sleepcall(10);
        //use ShowCharCall(...
    //}
    
    //call ExitCall with an exit code that is my_pid * 100

}
