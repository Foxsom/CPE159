// k-lib.c, 159

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"

// clear DRAM data block, zero-fill it
void Bzero(char *p, int bytes) {
   while(bytes--){
     *p++ = '\0';
//     cons_printf("Location %0x has value of %d\n", p, *p);
   }
}

int QisEmpty(q_t *p) { // return 1 if empty, else 0
   return (p->tail == 0);

}

int QisFull(q_t *p) { // return 1 if full, else 0
   return (p->tail == Q_SIZE);
}

// dequeue, 1st # in queue; if queue empty, return -1
// move rest to front by a notch, set empty spaces -1
int DeQ(q_t *p) { // return -1 if q[] is empty
	int pid, i;

   if(QisEmpty(p)) {
	   cons_printf("Panic: queue is empty, cannot DeQ!\n");
      return -1;
   }
   
   pid = p->q[0];
   p->tail--;

   for(i = 0; i < (p->tail) && i < Q_SIZE-1; i++){
     p->q[i] = p->q[i+1];
   }
   for(i = p->tail; i < Q_SIZE; i++) 
     p->q[i] = NONE;

   //p->q[p->tail] = -1;
   //p->tail = p->tail-1;
   //cons_printf("Dequeued: %d\n New Tail = %d\n", pid, p->tail+1);
 //  printf("DeQ pid = %d\n", pid);
   return pid;
}


// if not full, enqueue # to tail slot in queue
void EnQ(int to_add, q_t* p) {
  //printf("Queuing %d\n", to_add);
   if(QisFull(p)) {
      cons_printf("Panic: queue is full, cannot EnQ!\n");
      return;
   }

   p->q[p->tail] = to_add;

   p->tail++;
  // if(p->tail < Q_SIZE)
  //    p->q[p->tail] = NONE;
}

