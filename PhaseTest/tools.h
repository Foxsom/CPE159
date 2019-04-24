// k-lib.h, 159

#ifndef __K_LIB__
#define __K_LIB__

#include "k-type.h"

void Bzero(char *, int); // clear DRAM data block, zero-fill it
int QisEmpty(q_t *); // return 1 if empty, else 0
int QisFull(q_t *); // return 1 if full, else 0

// dequeue, 1st # in queue; if queue empty, return -1
// move rest to front by a notch, set empty spaces -1
int DeQ(q_t *); // return -1 if q[] is empty

// if not full, enqueue # to tail slot in queue
void EnQ(int, q_t *);

void MemCpy(char *, char *, int);
int StrCmp(char *, char *);
void Itoa(char *, int );


#endif