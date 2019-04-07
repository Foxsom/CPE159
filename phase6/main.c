// main.c, 159
// OS phase 1
//
// Team Name: gdb4life (Members: Tyler Fox, Zachary Derheim, Jesse Root)
#include "k-include.h"  // SPEDE includes
#include "k-entry.h"    // entries to kernel (TimerEntry, etc.)
#include "k-type.h"     // kernel data types
#include "k-lib.h"      // small handy functions
#include "k-sr.h"       // kernel service routines
#include "proc.h"       // all user process code here

// kernel data are all here:
int run_pid, vid_mux;                        // current running PID; if -1, none selected
int sys_centi_sec;
q_t pid_q, ready_q, sleep_q, mux_q;                 // avail PID and those created/ready to run
mux_t mux[MUX_SIZE];
pcb_t pcb[PROC_SIZE];               // Process Control Blocks
char proc_stack[PROC_SIZE][PROC_STACK_SIZE];   // process runtime stacks
struct i386_gate *intr_table;    // intr table's DRAM location


term_t term[TERM_SIZE] = {
{TRUE, TERM0_IO_BASE},
{TRUE, TERM1_IO_BASE}
};

void InitKernelData(void) {         // init kernel data
   int i;
      
   intr_table = get_idt_base();            // get intr table location
   sys_centi_sec = 0;

   Bzero((char *)&pid_q, sizeof(q_t));                      // clear 2 queues
   Bzero((char *)&ready_q, sizeof(q_t));
   Bzero((char *)&sleep_q, sizeof(q_t));
   Bzero((char *)&mux_q, sizeof(q_t));


   for(i=0; i<PROC_SIZE;i++){
     EnQ(i, &pid_q);                       // put all PID's to pid and mux queue
     EnQ(i, &mux_q);
   }
   run_pid= NONE;
//  cons_printf("InitKernelData: Complete\n");
}
void InitKernelControl(void) {      // init kernel control
   fill_gate(&intr_table[TIMER_INTR],(int)TimerEntry, get_cs(), ACC_INTR_GATE, 0 );                  // fill out intr table for timer
   fill_gate(&intr_table[GETPID_CALL],(int)GetPidEntry, get_cs(), ACC_INTR_GATE, 0 );
   fill_gate(&intr_table[SLEEP_CALL],(int)SleepEntry, get_cs(), ACC_INTR_GATE, 0 );
   fill_gate(&intr_table[SHOWCHAR_CALL],(int)ShowCharEntry, get_cs(), ACC_INTR_GATE, 0 );
   fill_gate(&intr_table[MUX_CREATE_CALL],(int)MuxCreateEntry, get_cs(), ACC_INTR_GATE, 0 );
  fill_gate(&intr_table[MUX_OP_CALL],(int)MuxOpEntry, get_cs(), ACC_INTR_GATE, 0 );

//Phase 4
  fill_gate(&intr_table[TERM0_INTR],(int)Term0Entry, get_cs(), ACC_INTR_GATE, 0 );
  fill_gate(&intr_table[TERM1_INTR],(int)Term1Entry, get_cs(), ACC_INTR_GATE, 0 );

   outportb(PIC_MASK, MASK);               // mask out PIC for timer
  // cons_printf("InitKernelControl: Complete\n");

//phase 6 JR
  fill_gate(&intr_table[FORK_CALL],(int) ForkEntry, get_cs(), ACC_INTR_GATE, 0);
  fill_gate(&intr_table[WAIT_CALL],(int) WaitEntry, get_cs(), ACC_INTR_GATE, 0);
  fill_gate(&intr_table[EXIT_CALL],(int) ExitEntry, get_cs(), ACC_INTR_GATE, 0);
}

void Scheduler(void) {      // choose run_pid
   if (run_pid > 0) 
      return; // OK/picked
 //  else
 //     run_pid = ;

   if (QisEmpty(&ready_q))
      run_pid = 0;     // pick InitProc
   else{
      pcb[0].state = READY;
                               //EnQ(0, &ready_q);
      run_pid = DeQ(&ready_q);
   }
   pcb[run_pid].run_count = 0;                    // reset run_count of selected proc
   pcb[run_pid].state = RUN;                    // upgrade its state to run
//cons_printf("Selected run_pid: %d\n", run_pid);
}

int main(void) {// OS bootstraps
   // cons_printf("Starting Main\n");
  
   InitKernelData();
   //cons_printf("Kernel Data Done\n");
  
   InitKernelControl();
   //cons_printf("Kernel Control Done\n");
    
   NewProcSR(InitProc); // create InitProc
   //cons_printf("Made new process\n");

   Scheduler();
   //cons_printf("Called Scheduler\n");

   Loader(pcb[run_pid].trapframe_p); // load/run it
   //cons_printf("Called Loader\n");

   return 0; // statement never reached, compiler asks it for syntax
}

void Kernel(trapframe_t *trapframe_p) {           // kernel runs
   char ch;

   pcb[run_pid].trapframe_p = trapframe_p; // save it
  
  //printf("Entry ID is %d\n", trapframe_p->entry_id);
   //TimerSR();                     // handle timer intr
   //printf("entry_id = %d\n",trapframe_p->entry_id);
  switch(trapframe_p->entry_id){
    case SLEEP_CALL:
      SleepSR(trapframe_p->eax);
      break;
    case GETPID_CALL:
      trapframe_p->eax = GetPidSR();
      break;
    case TIMER_INTR:
      TimerSR();
      break;
    case MUX_CREATE_CALL:
      trapframe_p->eax = MuxCreateSR(trapframe_p->eax);
      break;
    case MUX_OP_CALL:
      MuxOpSR(trapframe_p->eax, trapframe_p->ebx);
      break;
    case SHOWCHAR_CALL:
      ShowCharSR(trapframe_p->eax, trapframe_p->ebx, trapframe_p->ecx);
      break;
    case TERM0_INTR:
      //printf("Starting TERM0 Case\n");
      TermSR(0);
      //printf("TermSR(0) done\n");
      outportb(PIC_CONTROL, TERM0_DONE);
      break;
    case TERM1_INTR:
      //printf("Starting TERM1 Case\n");
      TermSR(1);      
      outportb(PIC_CONTROL, TERM1_DONE);
      break;
    // for phase 6 JR
    case FORK_CALL:
      // fill
      break;
    case WAIT_CALL:
      // fill
      break;
    case EXIT_CALL:
      // fill
      break;
  }

   if (cons_kbhit()) {            // check if keyboard pressed
      ch = cons_getchar();
      if (ch=='b')breakpoint();                    // 'b' for breakpoint
                                    // let's go to GDB
      else if (ch=='n') NewProcSR(UserProc);                     // 'n' for new process
     
   }
   Scheduler();    // may need to pick another proc
   Loader(pcb[run_pid].trapframe_p);
}




