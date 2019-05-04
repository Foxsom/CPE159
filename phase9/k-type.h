// k-type.h, 159

#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {UNUSED, READY, RUN, SLEEP, SUSPEND, ZOMBIE, WAIT, PAUSE} state_t;

typedef struct {
//unsigned int reg[8];
   unsigned int edi, esi, ebp, esp, ebx, 
   edx, ecx, eax, entry_id, eip, cs, efl;
} trapframe_t;

typedef struct {
   state_t state;                       // read in 1.html
   int run_count, total_count, wake_centi_sec;
   trapframe_t *trapframe_p;
   int ppid;

   //Phase 7 
   int sigint_handler;
   
   //Phase 9
   int main_table;
} pcb_t;                     

typedef struct {             // generic queue type
  int tail;                        // for a simple queue
  int q[Q_SIZE];
} q_t;

//Phase 3
typedef struct {
  int flag;
  int creater;
  q_t suspend_q;
  
} mux_t;

//Phase 4
typedef struct {
  int tx_missed;
  int io_base;
  int out_mux;
  q_t out_q;
  q_t in_q; //JR
  q_t echo_q; //JR
  int in_mux; //JR
} term_t;

//Phase 7 FOX
typedef void (*func_p_t2)(int);
#endif
 
