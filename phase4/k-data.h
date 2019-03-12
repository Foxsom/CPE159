// k-data.h, 159
// kernel data are all declared in main.c during bootstrap
// kernel .c code reference them as 'extern'

#ifndef __K_DATA__
#define __K_DATA__

#include "k-const.h"           // defines PROC_SIZE, PROC_STACK_SIZE
#include "k-type.h"            // defines q_t, pcb_t, ...

extern q_t ready_q, pid_q, sleep_q, mux_q;                            // prototype the rest...
extern pcb_t pcb[PROC_SIZE];
extern mux_t mux[MUX_SIZE];
extern char proc_stack[PROC_SIZE][PROC_STACK_SIZE];
extern int sys_centi_sec, run_pid, vid_mux;

//Phase 3
extern int vid_mux;
#endif                         // endif of ifndef
