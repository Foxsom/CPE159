// sys-call.c
// calls to kernel services

#include "k-const.h"
#include "k-type.h"
#include "k-sr.h"
#include "k-data.h"
#include "k-lib.h"
int GetPidCall(void) {
   int pid;

   asm("int %1\n\t"             // interrupt!
        "movl %%eax, %0\n\t"     // after, copy eax to variable 'pid'
       : "=g" (pid)         // output
       : "g" (GETPID_CALL)  // input
       : "eax"              // used registers
   );

   return pid;
}

void ShowCharCall(int row, int col, char ch) {
   asm("movl %0, %%eax;     // send in row via eax
       movl %1, %%ebx;            // send in col via ebx
       movl %2, %%ecx;            // send in ch via ecx
       int %3"             // initiate call, %3 gets entry_id
       :                    // no output
       :  "g" (row), "g"(col), "g" (ch), "g" (SHOWCHAR_CALL)
       : "eax", "ebx", "ecx" 
       );
}

void SleepCall(int centi_sec) {  // # of 1/100 of a second to sleep
   asm( "movl %0, %%eax;
        int %1"
        :
        : "g" (centi_sec), "g" (SLEEP_CALL)
        : "eax"
      );
}

//Phase 3
int MuxCreateCall(int flag){
  int muxID;
  asm("movl %1, %%eax;
      int %2;
      movl %%eax, %0"
      : "=g" (muxID)
      : "g" (flag), "g" (MUX_CREATE_CALL)
      : "eax"
  );
  return muxID;
}

void MuxOpCall(int mux_id, int opcode){
  asm("movl %0, %%eax;
      movl %1, %%ebx;
      int %2"
      :
      :"g" (mux_id), "g" (opcode), "g" (MUX_OP_CALL)
      : "eax", "ebx"    
  );  
}

void WriteCall(int device, char *str){
 int row, col, term_no;
 //printf("Write call started for device %d\n", device);
 row = GetPidCall();
 col = 0;
 
 if(device == STDOUT){
   while(*str != '\0'){
     ShowCharCall(row, col, *str);
     col++;
     str++;
   }
 }
 else{
   
   if(device==TERM0_INTR) term_no = 0;
   else term_no = 1;
   //printf("WriteCall Term Number is %d\n", term_no);
   while(*str != '\0'){
     MuxOpCall(term[term_no].out_mux, LOCK);
    // printf("Locked Mux\n");
     EnQ(*str, &term[term_no].out_q);
     //printf("Queued: %c\n", *str);
     if (device==TERM0_INTR) {
       //printf("Sending TERM0_INTR\n");
       asm("int $35");
     }
     else {
       //printf("Sending TERM1_INTR\n");
       asm("int $36");
     }
  //   printf("Interrupt Sent\n");
     str++;
     }
   }

}

void ReadCall (int device, char *str) {
    int term_no;

    if(device==TERM0_INTR) term_no = 0;
    else term_no =1;

    while(1) {
        MuxOpCall(term[term_no].in_mux, LOCK);   
        DeQ(&term[term_no].in_q);
        //need to set where strpoints to char
       // if() {
       //     return;
       //   }
        // advance str pointer & char count
       // if () {
            //set where str points to in NUL
       //     return;
       //   }
      
    }
  }
