// sys-call.h 159
//
#ifndef __SYS_CALL__
#define __SYS_CALL__

//#include "k-type.h"
//#include "k-const.h"
#include "k-data.h"
#include "tools.h"
extern int run_pid;
extern int GetPidCall(void);
extern void ShowCharCall(int row, int col, char ch);
extern void SleepCall(int centi_sec);
extern int MuxCreateCall(int flag);
extern void MuxOpCall(int mux_id, int opcode);
extern void WriteCall(int device, char *str);
extern void ReadCall(int device, char *str);
extern int ForkCall(void);
extern int WaitCall(void);
extern void ExitCall(int exit_code);

//Phase 7 FOX
extern void ExecCall(int code, int arg);
extern void SignalCall(int sig_num, int handler);

//Phase 8
extern void PauseCall(void);
extern void KillCall(int pid, int sig_num);
extern unsigned RandCall(void);
#endif
