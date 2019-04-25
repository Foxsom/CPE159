// k-entry.S, entries to kernel services, build process trapframe

#include "k-const.h"
#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#define K_STACK_SIZE 32768 //(PROC_STACK_SIZE * 8) // kernel has 32KB stack

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (32K x 1B)
.text

// load proc trapframe to run it
ENTRY(Loader)
   movl 4(%esp), %ebx
   movl %ebx, %esp
   popa
   add $4, %esp
   iret

// build proc trapframe, and go into kernel
ENTRY(TimerEntry)
  pushl $TIMER_INTR
  jmp PreKernel


ENTRY(GetPidEntry)
  pushl $GETPID_CALL
   jmp PreKernel

ENTRY(ShowCharEntry)
  pushl $SHOWCHAR_CALL
  jmp PreKernel
  
ENTRY(SleepEntry)
  pushl $SLEEP_CALL
  jmp PreKernel

//Phase 3
ENTRY(MuxCreateEntry)
  pushl $MUX_CREATE_CALL
  jmp PreKernel

ENTRY(MuxOpEntry)
  pushl $MUX_OP_CALL
  jmp PreKernel

ENTRY(Term0Entry)
  pushl $TERM0_INTR
  jmp PreKernel

ENTRY(Term1Entry)
  pushl $TERM1_INTR
  jmp PreKernel

ENTRY(ForkEntry)
  pushl $FORK_CALL
  jmp PreKernel

ENTRY(WaitEntry)
  pushl $WAIT_CALL
  jmp PreKernel

ENTRY(ExitEntry)
  pushl $EXIT_CALL
  jmp PreKernel

ENTRY(ExecEntry)
  pushl $EXEC_CALL
  jmp PreKernel

ENTRY(SignalEntry)
  pushl $SIGNAL_CALL
  jmp PreKernel

ENTRY(PauseEntry)
  pushl $PAUSE_CALL
  jmp PreKernel

ENTRY(KillEntry)
  pushl $KILL_CALL
  jmp PreKernel

ENTRY(RandEntry)
  pushl $RAND_CALL
  jmp PreKernel

PreKernel:                  // will have efl, cs, eip pushed by hardware
  pusha         //push all (8) registers
  movl %esp, %ebx
  movw $0x10, %ax                 // set kernel data segment registers
  mov %ax, %ds                    // set kernel data segment registers
  mov %ax, %es                    // set kernel data segment registers
  leal k_stack + K_STACK_SIZE, %esp  // set esp to top of kernel stack
  pushl %ebx                  //push ebx (onto the kernel stack)
  cld                 //clear the direction flag of efl
  call CNAME(Kernel)                //call C function Kernel
