// sys-call.c
// calls to kernel services

#include "k-const.h"
#include "k-type.h"
#include "k-sr.h"
#include "k-data.h"
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
   else if(device==TERM1_INTR) term_no = 1;
   term[term_no].out_mux = MuxCreateCall(1);             //May need to be moved and flag may be wrong
   while(*str != '\0'){
     MuxOpCall(term[term_no].out_mux, LOCK);
     EnQ(*str, &term[term_no].out_q);
     if (device==TERM0_INTR) asm("int $35");
     else asm("int $36");
     str++;
     }
   }

}
