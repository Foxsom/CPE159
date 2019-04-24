// k-sr.h, 159

#ifndef __K_SR__
#define __KSR__

#include "k-type.h"

void NewProcSR(func_p_t);
void CheckWakeProc(void);
void TimerSR(void);
int GetPidSR(void);
void ShowCharSR(int, int, char);
void SleepSR(int);
int MuxCreateSR(int);
void MuxOpSR(int, int);
void TermSR(int);
void TermTxSR(int);
void TermRxSR(int);

int ForkSR(void);
int WaitSR(void);
void ExitSR(int);

void ExecSR(int, int);
void SignalSR(int, int);
void WrapperSR(int, int, int);

#endif