// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use sys-calls

#include "k-const.h"   // LOOP
#include "sys-call.h"  // all service calls used below
#include "k-data.h"
#include "k-lib.h"
#include "k-include.h"
//term_t term[TERM_SIZE] = {{TRUE, TERM0_IO_BASE}, {TRUE, TERM1_IO_BASE}};
void InitTerm(int term_no){
  int i, j;

  Bzero((char *)&term[term_no].out_q, sizeof(q_t));
  term[term_no].out_mux = MuxCreateCall(Q_SIZE);

  outportb(term[term_no].io_base+CFCR, CFCR_DLAB);
  outportb(term[term_no].io_base+BAUDLO, LOBYTE(115200/9600));
  outportb(term[term_no].io_base+BAUDHI, HIBYTE(115200/9600));
  outportb(term[term_no].io_base+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);

  outportb(term[term_no].io_base+IER, 0);
  outportb(term[term_no].io_base+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);

  for(i=0; i<LOOP/2; i++) asm("inb $0x80");

  outportb(term[term_no].io_base+IER, IER_ERXRDY|IER_ETXRDY);
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");


  for(j=0; j<25; j++){
    outportb(term[term_no].io_base+DATA, '\n');
    for(i=0; i<LOOP/30; i++) asm("inb $0x80");
    outportb(term[term_no].io_base+DATA, '\r');
    for(i=0; i<LOOP/30; i++) asm("inb $0x80");
  }
  
  inportb(term[term_no].io_base);
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
}

void InitProc(void) {
   int i;
   vid_mux = MuxCreateCall(1);

   InitTerm(0);
   InitTerm(1);
   while(1) {
      ShowCharCall(0, 0, '.');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service

      ShowCharCall(0, 0, ' ');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service

   }
}

void UserProc(void) {
   
   int my_pid = GetPidCall();
   char str1[STR_SIZE] = "PID    process is running exclusively using the video display...";
   char str2[STR_SIZE] = "                                                                ";
   int which_term = my_pid%2==1? TERM0_INTR : TERM0_INTR;
   str1[4] = '0' + my_pid/10;
   str1[5] = '0' + my_pid%10;
    
   
   while(1) {

      MuxOpCall(vid_mux, LOCK);
      
      

      WriteCall(STDOUT, str1);  // show my PID
      WriteCall(which_term, str1);
      WriteCall(which_term, "\n\r");
      SleepCall(50);

                     // erasure
      WriteCall(STDOUT, str2);
      SleepCall(50);
      MuxOpCall(vid_mux, UNLOCK);
   }
}


