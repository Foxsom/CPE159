// proc.h, 159

#ifndef __PROC__
#define __PROC__
#include "sys-call.h"
  // prototype those in proc.c here
void InitProc(void);
void Delay(void);
void ShowChar(int row, int col, char ch);
void UserProc(void);
void InitTerm(int term_no);
void Aout(int device);
void Ouch(int device);
void Wrapper(int handler, int arg);
#endif
