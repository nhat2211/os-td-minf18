# Minf18
* TD OS Reporting
    - Devoir 0 
    - Members:
    - Vo Hung Son 
    - Tran Quang Nhat

# Multi-threading
## 1. Multi-threading in user programs

* Examine in details how NachOS threads work (kernel and user). How are allocated and initialized these threads?
    - Both kernel and user threads are initialized using Thread class. However, the user thread utilize space (with AddrSpace) and register (user-level CPU register state)

* Where is located the stack of a NachOS thread, as kernel thread?
    ```c++
    void Thread::StackAllocate(VoidFunctionPtr func, void *arg)
    {
        stack = (unsigned long *)AllocBoundedArray(StackSize * sizeof(unsigned long));
        ...
    }

    char * AllocBoundedArray(int size)
    {
        int pgSize = getpagesize();
        char *ptr = new char[pgSize * 2 + size];

        mprotect(ptr, pgSize, 0);
        mprotect(ptr + pgSize + size, pgSize, 0);
        return ptr + pgSize;
    }
    ```

### 1.1 Start your `putchar` program with trace options
```c++
int main()
{
	PutString("1234567890abcdefg\n");

	int n = GetChar();
	if(n != '\n') {
		PutChar(n);
		PutChar('\n');
	}
	
	Exit(2);	
}
```

### 1.3 Set up the system call interface
```c++
/** syacall.h */

#ifdef CHANGED
    int ThreadCreate(void f(void *arg), void *arg);
    void ThreadExit(void);
#endif CHANGED
```

```c++
/** start.S */ 

	.globl ThreadCreate
	.ent	ThreadCreate
ThreadCreate:
	addiu $2,$0,SC_ThreadCreate
	syscall
	j	$31
	.end ThreadCreate

	.globl ThreadExit
	.ent	ThreadExit
ThreadExit:
	addiu $2,$0,SC_ThreadExit
	syscall
	j	$31
	.end ThreadExit
```

* For what reason(s) could the creation of a thread fail?
    - Every thread takes some space in the stack, and the stack size is limited. Thus, in case there are many threads, it may not be able to create a new one.
    - Thread also consume memory, thus it may fail if there is not enough memory for creating and staring new thread (especially if there is memory leaked).
    - There may exist some limit on number of threads at OS level, once the limit reached, thread creation will fail.
    - ...

### 1.4 `do_ThreadCreate()` function

```c++
/** userthread.h */

#ifdef CHANGED
#include "thread.h"
extern int do_ThreadCreate (int f , int arg);
extern void do_ThreadExit();
#endif //CHANGED
```

```c++
/** exception.cc */

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#ifdef CHANGED
#include "userthread.h"
#endif
...
```

```c++
/** userthread.cc */

#ifdef CHANGED
#include "userthread.h"

int do_ThreadCreate(int f , int arg)
{
    Thread *newThread = new Thread ("user thread");
    if(newThread) {
        int *plop= new int[3];
        plop[0] = f;
        plop[1] = arg;        
        newThread->Start(StartUserThread, plop);
        return 0;
    }
    return -1;  
}
#endif // CHANGED
```
```c++
/** exception.cc */

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which)
    {
        case SyscallException:
        {
            switch (type)
            {
                ...
                case SC_ThreadCreate:
                {
                    DEBUG('s', "ThreadCreate\n");

        #ifdef CHANGED
                    int f = machine->ReadRegister(4);
                    int arg = machine->ReadRegister(5);                   
                    int ret = do_ThreadCreate(f, arg);

                    // Return -1 if the creation failed
                    machine->WriteRegister(2, ret);
        #endif
                    break;
                }
                case SC_ThreadExit:
                {
                    DEBUG('s', "ThreadExit\n");

        #ifdef CHANGED
                    do_ThreadExit();
        #endif

                    break;
                }
        #endif // CHANGED
                ...
            }
            ...
        }
        ...
    }
}
```

```make
# Makefile.common

USERPROG_O	:=	addrspace.o bitmap.o exception.o progtest.o console.o synchconsole.o \
			machine.o mipssim.o translate.o userthread.o
...
```

### 1.5 `StartUserThread()` function
```c++
/** userthread.cc */

#ifdef CHANGED
#define USER_THREAD_STACK_SIZE 256
#define STACK_PADDING 16

static void StartUserThread(void *thread_plop)
{
    int *plop = (int *)thread_plop;
    if (plop)
    {
        int f = plop[0];
        int arg = plop[1];        

        // Init stack and registers
        for (int i = 0; i < NumTotalRegs; i++)
            machine->WriteRegister(i, 0);

        // Set previous PC
        int pc = machine->ReadRegister(PCReg);
        machine->WriteRegister(PrevPCReg, pc);

        // Update PC and next PC
        machine->WriteRegister(PCReg, f);
        machine->WriteRegister(NextPCReg, f + 4);       

        // Set the page address for this thread
        unsigned int addr = currentThread->GetSpace()->GetNumPages() * PageSize - STACK_PADDING - (1 * USER_THREAD_STACK_SIZE - STACK_PADDING);
        machine->WriteRegister(StackReg, addr);

        // Write the arg
        machine->WriteRegister(4, arg);

        // Then run       
        machine->Run();

        delete plop; // free memory
    }
}
#endif // CHANGED
```

### 1.6 `do_ThreadExit()` functions
```c++
/** userthread.cc */

#ifdef CHANGED
void do_ThreadExit()
{
    // TODO: resource collection
    if(currentThread) {
        currentThread->Finish();
    }
}
#endif // CHANGED
```
* What should be done for its address space?
    - There should be resource collection before calling `currentThread->Finish()` to avoid memory leaks.


### 1.7 Demo
```c++
/** makethreads.c*/

#ifdef CHANGED
#include "syscall.h"

static int thread_done = 0;

void test(void *arg)
{
	PutString("Test PutChar():\n");
	PutChar('a');
	PutChar('\n');

	PutChar('\n');
	PutString("Test PutString():\n");
	PutString("arg:\n");
	PutString((char *)arg);
	PutChar('\n');

	PutString("Thread ended!\n");
	thread_done = 1;

    ThreadExit();
}

int main()
{
	char *arg = "Hello User Thread!\n";
	ThreadCreate(test, arg);

	while (thread_done == 0)
	{
		// Just wait
	}
}

#endif // CHANGED

```

## 2. Multiple Threads per process
## 2.1 Put `SynchPutChar()` and `SynchGetChar()` in critical section
```c++
/** synch.h */
class Lock : dontcopythis
{
#ifdef CHANGED
  Semaphore *m_semaphore; // semaphore instance
#endif
};
```

```c++
/** synch.cc */
Lock::Lock(const char *debugName)
{
#ifdef CHANGED
    name = debugName != NULL ? debugName : "lock";
    m_semaphore = new Semaphore(name, 1);
#endif
}

Lock::~Lock()
{
#ifdef CHANGED
    delete m_semaphore;
#endif
}
void Lock::Acquire()
{
#ifdef CHANGED
    m_semaphore->P();
#endif
}

void Lock::Release()
{
#ifdef CHANGED
    m_semaphore->V();
#endif
}
```

```c++
/** synchconsole.h */

#include "synch.h"

class SynchConsole : dontcopythis
{
    ...
private:
	Lock *m_readLock;  // Read Lock
	Lock *m_writeLock; // Write Lock
};

```
```c++
/** synchconsole.cc */

SynchConsole::SynchConsole(const char *in, const char *out)
{
	...
	m_readLock = new Lock("SynchConsole_ReadLock");
	m_writeLock = new Lock("SynchConsole_WriteLock");
}

SynchConsole::~SynchConsole()
{
    ...
	m_readLock->Release();
	m_writeLock->Release();

	delete m_readLock;
	delete m_writeLock;    
}

void SynchConsole::SynchPutChar(int ch)
{
	m_writeLock->Acquire();
	console->PutChar(ch);
	writeDone->P();
	m_writeLock->Release();
}

int SynchConsole::SynchGetChar()
{
	m_readLock->Acquire();
	readAvail->P();
	int c = console->GetChar();
	m_readLock->Release();
	return c;
}

void SynchConsole::SynchPutString(const char s[])
{
	m_writeLock->Acquire();
	int i = 0;
	while (s[i] != '\0')
	{
		console->PutChar(s[i++]);
		writeDone->P();
	}
	m_writeLock->Release();
}

void SynchConsole::SynchGetString(char *s, int n)
{
	m_readLock->Acquire();
	int c;
	while (n > 1)
	{
		readAvail->P();
		c = console->GetChar();
		*s++ = (char)c;
		if ((char)c == '\n')
			n = 0;
		else
			n--;
	}
	*s = '\0';
	m_readLock->Release();
}
```

```c++
/** makethreads.c */

static int thread_done = 0;

void test(void *arg)
{
    PutString("Test PutChar():\n");
    PutChar('a');
    PutChar('\n');

    PutChar('\n');
    PutString("Test PutString():\n");
    PutString("arg:\n");
    PutString((char *)arg);
    PutChar('\n');

    PutString("Thread ended!\n");
    thread_done = 1;

    ThreadExit();
}

int main()
{
    char *arg = "Hello User Thread!\n";
    ThreadCreate(test, arg);
    PutString("Started thread!\n");
    while (thread_done == 0)
    {
        // Just wait
        PutString("waiting!\n");
    }
    PutString("Thread ended in kernel!\n");
}
```

* Can you use two different locks?
    - Yes, we should use two locks, because read and write activities can be considered as 'non-critical' at hardware/kernel level, which can do in parallel to have better performance.
    - The implementation used two threads: one for reading and one for writing.

* Should SynchPutString() and SynchGetString() also be protected?
    - Yes, because those functions also access to Console which is a share resource.
    - Even if we modify those function to relatively SyncPutChar() and SyncGetChar(), the resource will not be in critical regions, but the content of the text print to console will not be in expected sequence. For example:
        ```c++
        /** synchconsole.cc */
    
        void SynchConsole::SynchPutString(const char s[])
        {      
            int i = 0;
            while (s[i] != '\0')
            {
                SynchPutChar(s[i++]); // sync
            }        
        }

        void SynchConsole::SynchGetString(char *s, int n)
        {        
            int c;
            while (n > 1)
            {
                c = SynchGetChar()); //sync
                *s++ = (char)c;
                if ((char)c == '\n')
                    n = 0;
                else
                    n--;
            }
            *s = '\0';
        }
        ```

* Does NachOS actually end if both the thread created and the initial thread are using ThreadExit?
    - No, the scheduler still running waiting for the new thread to be executed. Notice the Thread::Finish() and Thread::Sleep() functions:
    ```c++
    void Thread::Finish()
    {
        (void)interrupt->SetLevel(IntOff);
        ASSERT(this == currentThread);
        DEBUG('t', "Finishing thread \"%s\"\n", getName());
        ASSERT(threadToBeDestroyed == NULL);
        // End of addition
        threadToBeDestroyed = currentThread;
        Sleep(); // invokes SWITCH
        // not reached
    }

    void Thread::Sleep()
    {
        Thread *nextThread;

        ASSERT(this == currentThread);
        ASSERT(interrupt->getLevel() == IntOff);

        DEBUG('t', "Sleeping thread \"%s\"\n", getName());

        status = BLOCKED;
        while ((nextThread = scheduler->FindNextToRun()) == NULL)
            interrupt->Idle(); // no one to run, wait for an interrupt

        scheduler->Run(nextThread); // returns when we've been signalled
    }

    ```

## 2.2 Fix the termination by counting in `ThreadExit()`
```c++
/** userthread.cc */
static int threads_counter = 1; // at least main thread is running
Lock *thread_counter_lock = new Lock("ThreadCounter");

int do_ThreadCreate(int f, int arg, int f_e)
{   
    Thread *newThread = new Thread("user thread");
    if (newThread)
    {
        thread_counter_lock->Acquire();
        threads_counter++;
        thread_counter_lock->Release();
        return 0;
    }
    return -1;
}

void do_ThreadExit()
{   
    thread_counter_lock->Acquire();
    threads_counter--;
    thread_counter_lock->Release();

    if (threads_counter == 0)
    {       
        interrupt->Halt();
    }
    else
    {
        currentThread->Finish();
    }
}
```

* What would happen if the program started several threads instead of one?
    - The stack allocation is too basic, it only preserve space for one thread and without any guard/lock. So, it would crash with more than one threads created, even though it may work with small memory usage.
    ```c++
    /** makethreads.cc */
    #define N_THREADS 10

    void test(void *arg)
    {        
        PutString("arg:\n");
        PutString((char *)arg);
        ThreadExit();
    }

    int main()
    {
        char *arg = "Hello Thread!\n";
        int i, tid;
        for (i = 0; i < N_THREADS; i++)
        {
            if ((tid = ThreadCreate(test, (void *)i)) == -1)
            {
                PutString("Reached thread limit\n");
            }
        }

        PutString("Thread ended in kernel!\n");
        ThreadExit();
    }
    ```

## 2.3 Threads with for loop and stack
```c++
#define N_THREADS 10

int test_loop(void *arg)
{
	volatile int i;

	for (i = 0; i < 100; i++)
	{
		PutChar('a');
	}
	PutChar('\n');
	ThreadExit();
}

int main()
{
	char *arg = "Hello Thread!\n";
	int i, tid;
	for (i = 0; i < N_THREADS; i++)
	{
		ThreadCreate(test_loop, (void *)i);
	}

	ThreadExit();
}
```

* What happens if a program launches a large number of threads?
    - It will crash because of lacking memory. 

## 2.4 Review stack allocation mechanism
```c++
/** userthread.cc */

#define USER_THREAD_STACK_SIZE 256
#define STACK_PADDING 16
#define MAX_THREADS 10

static Lock *thread_lock = new Lock("thread_lock"); // Thread lock
static int g_threads[MAX_THREADS] = {0};            // store all available thread id
static BitMap *stackMap = new BitMap(MAX_THREADS);  // store info of stack slots
static int g_threadsIdx = 0;                        // index counter

int GetStackIdFromThread(int threadId)
{
    for (int i = 0; i < MAX_THREADS; i++)
    {
        if (g_threads[i] == threadId)
        {
            return i;
        }
    }
    return -1;
}

static void StartUserThread(void *thread_plop)
{
    int *plop = (int *)thread_plop;
    if (plop)
    {
        int f = plop[0];
        int arg = plop[1];        
        int idx = plop[2];

        // Init stack and registers
        for (int i = 0; i < NumTotalRegs; i++)
            machine->WriteRegister(i, 0);

        // Set previous PC
        int pc = machine->ReadRegister(PCReg);
        machine->WriteRegister(PrevPCReg, pc);

        // Update PC and next PC
        machine->WriteRegister(PCReg, f);
        machine->WriteRegister(NextPCReg, f + 4);

        // Set the page address for this thread, stack position align with the thread index to avoid issues
        unsigned int addr = currentThread->GetSpace()->GetNumPages() * PageSize - STACK_PADDING - (idx + 1) * USER_THREAD_STACK_SIZE - STACK_PADDING;
        machine->WriteRegister(StackReg, addr);

        // Write the arg
        machine->WriteRegister(4, arg);

        // Then run       
        machine->Run();

        delete plop; // free memory
    }
}

int do_ThreadCreate(int f, int arg)
{   
    thread_lock->Acquire();
    int idx = stackMap->Find();
    if (idx >= 0)
    {
        Thread *newThread = new Thread("user thread");
        if (newThread)
        {
            g_threads[idx] = (int)newThread;
            g_threadsIdx++;

            int *plop = new int[3];
            plop[0] = f;
            plop[1] = arg;          
            plop[2] = idx;
           
            newThread->Start(StartUserThread, plop);
            thread_lock->Release();
            return 0;
        }
    }

    thread_lock->Release();
    return -1;
}

void do_ThreadExit()
{ 
    thread_lock->Acquire();
    int idx = GetStackIdFromThread((int)currentThread);
    if (idx >= 0)
    {
        g_threads[idx] = 0;
        stackMap->Clear(idx);
    }
    g_threadsIdx--;
    thread_lock->Release();

    if (g_threadsIdx >= 0) // user thread counter
    {
        currentThread->Finish();
    }
    else // plus main thread
    {
        interrupt->Halt();
    }
}
```

```c++
/** makethreads.cc */

#include "syscall.h"

#define N_THREADS 10

int test_loop(void *arg)
{
	volatile int i;
	PutChar('\n');
	for (i = 0; i < 100; i++)
	{
		PutChar('a');
	}
	PutChar('\n');
	ThreadExit();
}

int main()
{
	char *arg = "Hello Thread!\n";
	int i, tid;
	for (i = 0; i < N_THREADS; i++)
	{
		if ((tid = ThreadCreate(test_loop, (void *)i)) == -1)
		{
			PutString("Reached thread limit\n");
		}
	}
	ThreadExit();
}
```

## 3. Automatic Termination
The `RetAddrReg` register hold the return address from the procedure call, thus we can just set the register to the address of `ThreadExit`, it will call the exit automatically.

```c++
/** start.S */
	.globl ThreadCreate
	.ent	ThreadCreate
ThreadCreate:
	addiu $2,$0,SC_ThreadCreate
	addiu $6,$0,ThreadExit /* $4=>f, $5=>arg, $6=>f_exit*/ 
	syscall
	j	$31
	.end ThreadCreate
```
```c++
/** exception.cc */

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which)
	{
        case SyscallException:
        {
            switch (type)
            {
                ...
                case SC_ThreadCreate:
                {
                    DEBUG('s', "ThreadCreate\n");
                    int f = machine->ReadRegister(4);
                    int arg = machine->ReadRegister(5);
                    int f_exit = machine->ReadRegister(6);
                    int ret = do_ThreadCreate(f, arg, f_exit);                    
                    machine->WriteRegister(2, ret); // return code
                    break;
                }
            }
            ...
        }
        ...
    }
    ...
}
```
```c++
/** userthread.h */
extern int do_ThreadCreate(int f, int arg, int f_exit);
```

```c++
/** userthread.cc */
static void StartUserThread(void *thread_plop)
{
    int *plop = (int *)thread_plop;
    if (plop)
    {
        int f = plop[0];
        int arg = plop[1];
        int f_exit = plop[2];
        int idx = plop[3];

        // Init stack and registers
        for (int i = 0; i < NumTotalRegs; i++)
            machine->WriteRegister(i, 0);

        // Set previous PC
        int pc = machine->ReadRegister(PCReg);
        machine->WriteRegister(PrevPCReg, pc);

        // Update PC and next PC
        machine->WriteRegister(PCReg, f);
        machine->WriteRegister(NextPCReg, f + 4);
        machine->WriteRegister(RetAddrReg, f_exit);

        // Set the page address for this thread, stack position align with the thread index to avoid issues
        unsigned int addr = currentThread->GetSpace()->GetNumPages() * PageSize - STACK_PADDING - (idx + 1) * USER_THREAD_STACK_SIZE - STACK_PADDING;
        machine->WriteRegister(StackReg, addr);

        // Write the arg
        machine->WriteRegister(4, arg);

        // Then run       
        machine->Run();

        delete plop; // free memory
    }
}

int do_ThreadCreate(int f, int arg, int f_exit)
{
    thread_lock->Acquire();
    int idx = stackMap->Find();
    if (idx >= 0)
    {
        Thread *newThread = new Thread("user thread");
        if (newThread)
        {
            g_threads[idx] = (int)newThread;
            g_threadsIdx++;

            int *plop = new int[4];
            plop[0] = f;
            plop[1] = arg;
            plop[2] = f_exit;
            plop[3] = idx;
           
            newThread->Start(StartUserThread, plop);
            thread_lock->Release();
            return 0;
        }
    }

    thread_lock->Release();
    return -1;
}
```
```c++
/** makethreads.cc */
int test_loop(void *arg)
{
	volatile int i;
	PutChar('\n');
	for (i = 0; i < 100; i++)
	{
		PutChar('a');
	}
	PutChar('\n');	
}

int main()
{
	char *arg = "Hello Thread!\n";
	int i, tid;
	for (i = 0; i < N_THREADS; i++)
	{
		if ((tid = ThreadCreate(test_loop, (void *)i)) == -1)
		{
			PutString("Reached thread limit\n");
		}
    }
    ThreadExit(); // this is still needed for main thread
}

```
