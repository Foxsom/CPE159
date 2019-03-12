// k-sr.h, 159

#ifndef __K_SR__
#define __K_SR__

//#include "k-sr.c"

void NewProcSR(func_p_t p);
void TimerSR(void);
int GetPidSR(void);
void SleepSR(int centi_sec);
void CheckWakeProc(void);

//Phase 3
void ShowCharSR(int row, int col, char ch);
int MuxCreateSR(int flag);
void MuxOpSR(int mux_id, int opcode);
#endif
