// proc.h, 159

#ifndef __PROC__
#define __PROC__

//void Delay(void); // delay CPU for half second by 'inb $0x80'
//void ShowChar(int, int, char); // show ch at row, col
void InitProc(void);
void UserProc(void);
void InitTerm(int);

void Aout(int);
void Ouch(int);
void Wrapper(int, int);

#endif