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
  Bzero((char *)&term[term_no].in_q, sizeof(q_t));
  Bzero((char *)&term[term_no].echo_q, sizeof(q_t));

  //printf("Creating term Mux\n");
  term[term_no].out_mux = MuxCreateCall(Q_SIZE);
  term[term_no].in_mux = MuxCreateCall(0);

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
   int my_pid = GetPidCall();

   char str1[STR_SIZE] = "PID    > ";
   char str2[STR_SIZE];
   
   int device = my_pid%2==1? TERM0_INTR : TERM1_INTR;
   //printf("PID %d interrupt is %d\n", my_pid, which_term);
  // if(my_pid%2==1) which_term = TERM0_INTR;
  // else which_term = TERM1_INTR;
   str1[4] = '0' + my_pid/10;
   str1[5] = '0' + my_pid%10;
   
   while(1) {

      //MuxOpCall(vid_mux, LOCK);
      
      
//      printf("writing to STDOUT %d\n", STDOUT);
      //WriteCall(STDOUT, str1);  // show my PID
 //     printf("wrinting to term %d\n", which_term);
      printf("write to term %d\n", device);
      WriteCall(device, str1);
      ReadCall(device, str2);
      WriteCall(STDOUT, str2);
      //SleepCall(50);

                     // erasure
 //     printf("Erasing text from display\n");
      //WriteCall(STDOUT, str2);
      //SleepCall(50);
      //MuxOpCall(vid_mux, UNLOCK);
   }
}


