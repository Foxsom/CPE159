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
  //printf("MuxCreateCall flag = %d\n");
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
    int term_no, charCount;
    char c;

    if(device==TERM0_INTR) term_no = 0;
    else term_no =1;
    
    charCount = 0;

    while(1) {
        MuxOpCall(term[term_no].in_mux, LOCK);
	//printf("Attempting to deQ a char\n");   
        c = DeQ(&term[term_no].in_q);
        *str = c;

        if(c=='\0') {
            return;
          }

        str++;
	charCount++;

        if (charCount==STR_SIZE) {
            *str = '\0';
            return;
          }
      
    }
  }

int ForkCall(void){
	int pid;
	asm("int %1;
      movl %%eax, %0"
      : "=g" (pid)
      : "g" (FORK_CALL)
      : "eax"
  );
	return pid;	
}

int WaitCall(void){
	int exitCode;
        asm("int %1;
      movl %%eax, %0"
      : "=g" (exitCode)
      : "g" (WAIT_CALL)
      : "eax"
  );
        return exitCode;
}



void ExitCall(int exit_code){
	asm("movl %1, %%eax;
	int %0"
      : 
      : "g" (EXIT_CALL), "g" (exit_code)
      : "eax"
  );
}

void ExecCall(int code, int arg){


}

void SignalCall(int sig_num, int handler){


}
