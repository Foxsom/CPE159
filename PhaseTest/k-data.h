// k-data.h, 159
// kernel data are all declared in main.c during bootstrap
// kernel .c code reference them as 'extern'

#ifndef __K_DATA__
#define __K_DATA__

#include "k-const.h"           // defines PROC_SIZE, PROC_STACK_SIZE
#include "k-type.h"            // defines q_t, pcb_t, ...

extern int run_pid;            // PID of running process
extern q_t ready_q, pid_q;                            // prototype the rest...
extern pcb_t pcb[PROC_SIZE];
extern char proc_stack[PROC_SIZE][PROC_STACK_SIZE];
void Kernel(trapframe_t *);
extern void InitProc(void);
extern trapframe_t * p;
extern int sys_centi_sec;
extern q_t sleep_q;
extern mux_t mux[MUX_SIZE];
extern q_t mux_q;
extern int vid_mux;
extern term_t term[TERM_SIZE];
extern int page_user[PAGE_NUM];

#endif                         // endif of ifndef