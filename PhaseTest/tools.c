// k-lib.c, 159

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"

// clear DRAM data block, zero-fill it
void Bzero(char *p, int bytes) {
        int i;
        for(i = 0; i < bytes; i++)
        {
                *(p + i) = 0;
        }
}

int QisEmpty(q_t *p) { // return 1 if empty, else 0
        if(p->tail == 0)
                return 1;
        else
                return 0;
}

int QisFull(q_t *p) { // return 1 if full, else 0
        if(p->tail == Q_SIZE)
                return 1;
        else
                return 0;
}

// dequeue, 1st # in queue; if queue empty, return -1
// move rest to front by a notch, set empty spaces -1
int DeQ(q_t *p) { // return -1 if q[] is empty
        int dq_value;
        int i;

        if(QisEmpty(p)) {
                return -1;
        }

        dq_value = p->q[0];
        for(i = 0; i < Q_SIZE; i++)
        {
                if(i < p->tail - 1)
                        p->q[i] = p->q[i+1];
                else
                        p->q[i] = -1;
        }
	p->tail--;
        return dq_value;
}

// if not full, enqueue # to tail slot in queue
void EnQ(int to_add, q_t *p) {
        if(QisFull(p)) {
               cons_printf("Panic: queue is full, cannot EnQ!\n");
               breakpoint();
        }
        p->q[p->tail] = to_add;
        p->tail++;
}

//copies 'size' of bytes from 'src' to 'dst'
void MemCpy(char *dst, char *src, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		dst[i] = src[i];
	}
}

//returns TRUE if str1 and str2 ae the same, otherwise, 0
int StrCmp(char *str1, char *str2){ 

   while(1) {
      if(*str1 != *str2) {
         return FALSE;
      }
       
      if(*str2 == '\0' && *str1 == '\0') { //added str1 condition
         return TRUE;
      }else if(*str2 == '\0' || *str1 == '\0'){
	 return FALSE;
      }
      str1++;
      str2++;
   }
}

//converts an unsigned decimal number x < 1000000 into str
void Itoa(char *str, int x)
{
        int i;
        int digits = 0;
        int temp = x;

        if(x >= 1000000)
                return;

        do
        {
                temp /= 10;
                digits++;
        }while(temp != 0);

        temp = 10;

        for(i = 0; i < digits; i++)
        {
                *(str + digits - 1 - i) = x % temp + 48;
                x /= 10;
        }

        *(str + digits) = '\0';

        return;
}
