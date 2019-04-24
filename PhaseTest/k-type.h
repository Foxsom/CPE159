// k-type.h, 159

#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef void (*func_p_t2)(int); //in order to point to a function of this form: void Ouch(int) (Yet, typecasting to 'int' will be easier to program.)

typedef enum {UNUSED, READY, RUN, SLEEP, SUSPEND, ZOMBIE, WAIT} state_t;

typedef struct {
   unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
   unsigned int entry_id;
   unsigned int eip;
   unsigned int cs;
   unsigned int efl;
} trapframe_t;

typedef struct {
	int run_count;                       // read in 1.html
	state_t state;
	int total_count;
	trapframe_t * trapframe_p;
	int wake_centi_sec;
	int ppid; //record parent PID when the process is created by ForkSR()
	int sigint_handler;
} pcb_t;                     

typedef struct {             // generic queue type
	int q[Q_SIZE];                        // for a simple queue
	int tail;
} q_t;

typedef struct{
	int flag;	//max # of processes to enter
	int creater;	//requester/owning PID
	q_t suspend_q;	//suspended PID's
}mux_t;

typedef struct{
	int tx_missed;	// when initialized or after output last char
	int io_base;	// terminal port I/O base #
	int out_mux;	// flow-control mux
	int in_mux;	//flow-control in_q
	q_t in_q; //to buffer term KB input
	q_t echo_q; // to echo back to term
	q_t out_q;	//// characters to send to terminal buffered here
}term_t;

#endif