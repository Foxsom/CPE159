// sys-call.h 159
//
#ifndef __SYS_CALL__
#define __SYS_CALL__

#include "k-type.h"
#include "k-const.h"
#include "sys-call.c"
extern int run_pid;
int GetPidCall(void);
void ShowCharCall(int row, int col, char ch);
void SleepCall(int centi_sec);
int MuxCreateCall(int flag);
void MuxOpCall(int mux_id, int opcode);
void WriteCall(int device, char *str);
#endif
