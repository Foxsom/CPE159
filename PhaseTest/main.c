// main.c, 159
// OS phase 4
//
// Team Name: get_me_out (Members: Cody Rider, Nick Rentschler)

#include "k-include.h"  // SPEDE includes
#include "k-entry.h"    // entries to kernel (TimerEntry, etc.)
#include "k-type.h"     // kernel data types
#include "tools.h"      // small handy functions
#include "k-sr.h"       // kernel service routines
#include "proc.h"       // all user process code here

// kernel data are all here:
int run_pid;                        // current running PID; if -1, none selected
q_t pid_q, ready_q;                 // avail PID and those created/ready to run
pcb_t pcb[PROC_SIZE];               // Process Control Blocks
char proc_stack[PROC_SIZE][PROC_STACK_SIZE];   // process runtime stacks
struct i386_gate *intr_table;    // intr table's DRAM location
int sys_centi_sec;		//system time in centi-sec, initialize it 0
q_t sleep_q;			//sleeping proc PID's queued in here
mux_t mux[MUX_SIZE];		// kernel has these mutexes to spare
q_t mux_q;			// mutex ID's are initially queued here
int vid_mux;			// for video access control
term_t term[TERM_SIZE] = {{TRUE, TERM0_IO_BASE}, {TRUE, TERM1_IO_BASE}};
int page_user[PAGE_NUM];

void InitKernelData(void) {         // init kernel data
	int i;

	sys_centi_sec = 0;      

	intr_table = get_idt_base();            // get intr table location

	Bzero((char *)&ready_q, sizeof(q_t));                      // clear 4 queues
	Bzero((char *)&pid_q, sizeof(q_t));
	Bzero((char *)&sleep_q, sizeof(q_t));
	Bzero((char *)&mux_q, sizeof(q_t));

	for(i=0; i < PAGE_NUM; i++)
	{
		page_user[i] = NONE;
	}	

	for(i=0; i < Q_SIZE; i++)                        // put all PID's to pid queue & all mux ID's in mux queue
	{
		EnQ(i, &pid_q);
		EnQ(i, &mux_q);
	}

	run_pid = NONE;//set run_pid to NONE
}

void InitKernelControl(void) {      // init kernel control
	fill_gate(&intr_table[TIMER_INTR], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0);                  // fill out intr table for timer
	fill_gate(&intr_table[GETPID_CALL], (int)GetPidEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[SHOWCHAR_CALL], (int)ShowCharEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[SLEEP_CALL], (int)SleepEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[MUX_CREATE_CALL], (int)MuxCreateEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[MUX_OP_CALL], (int)MuxOpEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[TERM0_INTR], (int)Term0Entry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[TERM1_INTR], (int)Term1Entry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[FORK_CALL], (int)ForkEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[WAIT_CALL], (int)WaitEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[EXIT_CALL], (int)ExitEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[EXEC_CALL], (int)ExecEntry, get_cs(), ACC_INTR_GATE, 0);
	fill_gate(&intr_table[SIGNAL_CALL], (int)SignalEntry, get_cs(), ACC_INTR_GATE, 0);
	outportb(PIC_MASK, MASK);                   // mask out PIC for timer
}

void Scheduler(void) {      // choose run_pid
	if(run_pid > 0) //run_pid is greater than 0, just return; // OK/picked
	{
		return;		
	}

	if(QisEmpty(&ready_q)) //ready_q is empty: pick 0 as run_pid     // pick InitProc
	{
		run_pid = 0;
	}
	else
	{
		pcb[0].state = READY;	//change state of PID 0 to ready
		run_pid = DeQ(&ready_q);	//dequeue ready_q to set run_pid
	}
	pcb[run_pid].run_count = 0;                    // reset run_count of selected proc
	pcb[run_pid].state = RUN;                    // upgrade its state to run
}

int main(void) {                          // OS bootstraps
	InitKernelData(); //call to initialize kernel data
	InitKernelControl(); //call to initialize kernel control

	NewProcSR(InitProc); //call NewProcSR(InitProc) to create it  // create InitProc
	Scheduler(); //call Scheduler()
	Loader(pcb[run_pid].trapframe_p); //call Loader(pcb[run_pid].trapframe_p); // load/run it

   return 0; // statement never reached, compiler asks it for syntax
}

void Kernel(trapframe_t *trapframe_p) {           // kernel runs
	char ch;

	pcb[run_pid].trapframe_p = trapframe_p; // save it

	switch(trapframe_p->entry_id)
	{
		case TIMER_INTR:
			TimerSR(); // handle timer intr
			break;
		case GETPID_CALL:
			trapframe_p->eax = GetPidSR();
			break;
		case SHOWCHAR_CALL:
			ShowCharSR(trapframe_p->eax, trapframe_p->ebx, trapframe_p->ecx);
			break;
		case SLEEP_CALL:
			SleepSR(trapframe_p->eax);
			break;
		case MUX_CREATE_CALL:
			trapframe_p->eax = MuxCreateSR(trapframe_p->eax);
			break;
		case MUX_OP_CALL:
			MuxOpSR(trapframe_p->eax, trapframe_p->ebx);
			break;
		case TERM0_INTR:
			TermSR(0);
			outportb(PIC_CONTROL, TERM0_DONE_VAL);
			break;
		case TERM1_INTR:
			TermSR(1);
			outportb(PIC_CONTROL, TERM1_DONE_VAL);
			break;
		case FORK_CALL:
			trapframe_p->eax = ForkSR();
			break;
		case WAIT_CALL:
			trapframe_p->eax = WaitSR();
			break;
		case EXIT_CALL:
			ExitSR(trapframe_p->eax);
			break;
		case EXEC_CALL:
			ExecSR(trapframe_p->eax, trapframe_p->ebx);
			break;
		case SIGNAL_CALL:
			SignalSR(trapframe_p->eax, trapframe_p->ebx);
			break;
		default:
			cons_printf("Panic: Invalid entry_id!\n");
			breakpoint();
	}

	if( cons_kbhit() ) {            // check if keyboard pressed
		ch = cons_getchar();
		if(ch == 'b')                     // 'b' for breakpoint
			breakpoint();                           // let's go to GDB
		if(ch == 'n')                     // 'n' for new process
			NewProcSR(UserProc);     // create a UserProc
	}
	Scheduler();    // may need to pick another proc
	Loader(pcb[run_pid].trapframe_p);
}