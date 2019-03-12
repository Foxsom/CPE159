// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use sys-calls

#include "k-const.h"   // LOOP
#include "sys-call.h"  // all service calls used below
#include "k-data.h"

void InitProc(void) {
   int i;
   vid_mux = MuxCreateCall(1);
   while(1) {
      ShowCharCall(0, 0, '.');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service

      ShowCharCall(0, 0, ' ');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service

   }
}

void UserProc(void) {
   int my_pid;
   char output[40];
    
   
   while(1) {
      MuxOpCall(vid_mux, LOCK);
      my_pid = GetPidCall();
      
      sprintf(output, "PID %d process is running exclusively using the video display...", my_pid);
      

      WriteCall(STDOUT, output);  // show my PID
      
      SleepCall(50);

                     // erasure
      WriteCall(STDOUT, "                                                                  ");
      SleepCall(50);
      MuxOpCall(vid_mux, UNLOCK);
   }
}


