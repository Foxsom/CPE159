// k-sr.h, 159

#ifndef __K_SR__
#define __K_SR__


void TermSR(int term_no);       //<---------------Phase 4
void NewProcSR(func_p_t p);
void TimerSR(void);
int GetPidSR(void);
void SleepSR(int centi_sec);
void CheckWakeProc(void);

//Phase 3
void ShowCharSR(int row, int col, char ch);
int MuxCreateSR(int flag);
void MuxOpSR(int mux_id, int opcode);

//Phase 4
void TermTxSR(int term_no);
void TermRxSR(int term_no);


//Phase 6 FOX
int ForkSR(void);
int WaitSR(void);
void ExitSR(int exit_code);

//Phase 7 FOX
void ExecSR(int code, int arg);
void SignalSR(int sig_num, int handler);
void WrapperSR(int pid, int handler, int arg);

//Phase 8
void PauseSR(void);
void KillSR(int pid, int sig_num);
unsigned RandSR(void);
#endif
