// k-lib.h, 159

#ifndef __K_LIB__
#define __K_LIB__

//#include "k-lib.c"

     // prototype those in k-lib.c here
void Bzero(char *p, int bytes);      
int QisEmpty(q_t *p);
int QisFull(q_t *p);
int DeQ(q_t *p);
int EnQ(int to_add, q_t *p);

#endif
