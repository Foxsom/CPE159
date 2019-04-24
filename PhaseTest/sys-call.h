// sys-call.h

#ifndef __sys_call__
#define __sys_call__

//insert any dependencies


//prototypes..
int GetPidCall(void);
void ShowCharCall(int, int, char);
void SleepCall(int);
int MuxCreateCall(int);
void MuxOpCall(int, int);
void WriteCall(int, char *);
void ReadCall(int, char *);
int ForkCall(void);
int WaitCall(void);
void ExitCall(int);
void ExecCall(int, int);
void SignalCall(int, int);

#endif