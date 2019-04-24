// sys-call.c
// calls to kernel services

#include "k-const.h"
#include "k-data.h"
#include "tools.h"

int GetPidCall(void) {
   int pid;

   asm("int %1;             // interrupt!
        movl %%eax, %0"     // after, copy eax to variable 'pid'
       : "=g" (pid)         // output
       : "g" (GETPID_CALL)  // input
       : "eax"              // used registers
	);
	return pid;
}

void ShowCharCall(int row, int col, char ch) {
   asm("movl %0, %%eax;     // send in row via eax
        movl %1, %%ebx;            // send in col via ebx
        movl %2, %%ecx;             // send in ch via ecx
        int %3"             // initiate call, %3 gets entry_id
       :                    // no output
       : "g" (row), "g" (col), "g" (ch), "g" (SHOWCHAR_CALL)
       : "eax", "ebx", "ecx"         // affected/used registers
   );
}

void SleepCall(int centi_sec) {  // # of 1/100 of a second to sleep
   asm( "movl %0, %%eax;
	int %1"
	:			// no output
	: "g" (centi_sec), "g" (SLEEP_CALL) // input
	: "eax"
	);
}

int MuxCreateCall(int flag)
{

	int mux_id;

 	asm
	(	"movl %2, %%eax;	//move flag into eax
        	int %1;
		movl %%eax, %0"     // after, copy eax to variable 'mux_id'
       		: "=g" (mux_id)         // output
       		: "g" (MUX_CREATE_CALL), "g" (flag)  // input
       		: "eax"              // used registers
        );

        return mux_id;

}

void MuxOpCall(int mux_id, int opcode)
{
	asm(
		"movl %0, %%eax;	//move mux_id into eax
		movl %1, %%ebx;		// move opcode into ebx
        	int %2"
        	:                       // no output
        	: "g" (mux_id), "g" (opcode), "g" (MUX_OP_CALL) // input
        	: "eax", "ebx"
        );

}

void WriteCall(int device, char *str)
{
	int row;
	int col;
	row = GetPidCall();	//get my PID by service call
				//calc my row number
	col = 0;		//column is zero to begin with
	if(device ==  STDOUT)	//if device is STDOUT
	{
		while(*str != '\0')	//while what str points to is not a null character
		{
			ShowCharCall(row, col, *str);	//use an existing service call to show this character, at row and column
			str++;				//increment the str pointer and the column position
			col++;
		}
	}
	else	//device is a terminal
	{
		int term_no;
		if(device == TERM0_INTR)
			term_no = 0;
		else
		{
			term_no = 1;
		}

		while(*str != '\0')
		{
			MuxOpCall(term[term_no].out_mux, LOCK);
			EnQ(*str, &term[term_no].out_q);

                        if(device == TERM0_INTR)
                        {
                                asm(    "int %0"
                                        :       //no output
                                        : "g" (TERM0_INTR) //input
                                );
                        }
                        else
                        {
                                asm(    "int %0"
                                        :       //no output
                                        : "g" (TERM1_INTR) //input
                                );
                        }

			str++;
		}
	}
}

void ReadCall(int device, char *str)
{

	int term_no;	
	int char_count = 0;
	char ch;

	if(device == TERM0_INTR)
	{
		term_no = 0;
	}
	else
	{
		term_no = 1;
	}
	
	while(1)
	{
		MuxOpCall(term[term_no].in_mux, LOCK);
		ch = DeQ(&term[term_no].in_q);

		*str = ch;

		if(ch == '\0')
		{
			return;
		}
		
		str++;
		char_count++;
		if(char_count == STR_SIZE)
		{
			*str = '\0';
			return;
		}
	}
}

//returns the PID of the new child process to the calling (parent process) or NONE if the call fails. If succeeded, the child gets 0
int ForkCall(void)
{
	int c_pid;

 	asm(    "int %1;
		movl %%eax, %0"
                : "=g" (c_pid)    //no output
		: "g" (FORK_CALL) //input
		: "eax"
	);
	return c_pid;
}

//returns the exit code from the child process (as it exits)
int WaitCall(void)
{
	int exit_code;
	
	asm(	"int %1;
		movl %%eax, %0" 
		: "=g" (exit_code) //output
		: "g" (WAIT_CALL)
		: "eax"
	);
	return exit_code;
}

//called by an exiting process, an exit code is given reurn to its parent process
void ExitCall(int exit_code)
{
	asm(	"movl %0, %%eax;
		int %1"
		:
		: "g" (exit_code), "g" (EXIT_CALL)
		: "eax"
	);
}

void ExecCall(int code, int arg)
{
	asm(	"movl %0, %%eax;
		movl %1, %%ebx;
		int %2"
		:	//no output
		: "g" (code), "g" (arg), "g" (EXEC_CALL) //input
		: "eax", "ebx" //registers
	);
}

void SignalCall(int sig_num, int handler)
{
	asm(	"movl %0, %%eax;
		movl %1, %%ebx;
		int %2"
		:	//no output
		: "g" (sig_num), "g" (handler), "g" (SIGNAL_CALL) //input
		: "eax", "ebx"
	);
}