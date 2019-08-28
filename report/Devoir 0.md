# MINF18
* TD OS Reporting 
    - Devoir 0 
    - Members:
    - Vo Hung Son 
    - Le Dinh Than
    - Tran Quang Nhat

## 2. Source code reading
### 2.1 Simulation principles
Give an example source file of a MIPS user program, and an example source file of the NachOS kernel. What is the programming language used each time?
Answer:
Source file of a MIPS user program: Assembly Language
1. .data
2. prompt1: .asciiz "Enter the first number: "
3. prompt2: .asciiz "Enter the second number: "
4. menu: .asciiz "Enter the number associated with the operation you want performed: 1
=> add, 2 => subtract or 3 => multiply: "
5. resultText: .asciiz "Your final result is: "
The programming language used each time is C++ language.
The code of `./userprog/nachos -x ./test/halt`
* File [`./threads/main.cc`](../code/threads/main.cc):
    ```c++
    int main(int argc, char **argv)
    {
        ...
        for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount)
        {
            ...
            if (!strcmp(*argv, "-x"))
            { // run a user program
                ASSERT(argc > 1);
                StartProcess(*(argv + 1));
                argCount = 2;
            }
            ...
        }
        ...
    }
    ``` 
* File [`./userprog/progtest.cc`](../code/userprog/progtest.cc)
    ```c++
    void StartProcess (char *filename)
    {
        OpenFile *executable = fileSystem->Open (filename);
        AddrSpace *space;

        if (executable == NULL)
        {
        printf ("Unable to open file %s\n", filename);
        return;
        }
        space = new AddrSpace (executable);
        currentThread->space = space;

        delete executable;		// close file

        space->InitRegisters ();	// set the initial register values
        space->RestoreState ();	// load page table register

        machine->Run ();		// jump to the user program
        ASSERT (FALSE);		// machine->Run never returns;
        // the address space exits
        // by doing the syscall "exit"
    }
    ```  
* File [`./test/halt.c`](../code/test/halt.c)
    ```c++
    int main ()
    {
        Halt ();

        /* not reached */
        return 0;
    }
    ``` 
### 2.2 System initialization

* How is this first kernel thread created?
    - Answer:
    - Step 1:Call into Start method.
    - Step 2: In the Start method call into StackAllocate method
    - Step 3: In the StackAllocate mehod that we init a stack with size of long
    - step 4: Register stack by valgrind_id.
    - Step 5: check constant value to work in range (low addresses to high addresses or opposite) Step 6: SetupThreadState that it is      called - be StartupPCState of machine.
    - Step 7: Register InitialPCState of machine state by func with long value.
    - Step 8: Register InitialArgState of machine state by arg with long value.
    - Step 9: Register WhenDonePCState of machine state by ThreadFinish method with long value.
    - Step 10: Set IntStatus oldLevel = interrupt->SetLevel (IntOff)
    - Step 11: Run the thread by code line scheduler->ReadyToRun (this);
    - So the first kernel thread is the main thread that was created by operating system.
* Where does its stack and its registers come from?
The stack and registers come from the main thread which is called by the real operating system, when allocate the program and start the `main()` method.
* What is the (future) role of the data structure allocated by the instruction:
HOST_SNAKE: HP stack works from low addresses to high addresses;HP requires 64-byte frame marker
#else: other archs stack works from high addresses to low addresses HOST_SPARC:SPARC stack must contains at least 1 activation record to start with. HOST_PPC
HOST_i386: -4 for the return address
HOST_x86_64: -8 for the return address
HOST_MIPS
    ``` c++ 
    currentThread = new Thread("main"); 
    ```
The role of the currentThread is to keep the handle of the main thread so that we can call `sleep()` lately if we want to reserve CPU power to other threads.

* Why is it necessary to call the Start method for the next kernel threads? (focus into threads/thread.h and threads/thread.cc)
Because in multiple threading program, only the main thread initialized by the OS caller, other threads are initialized by the program's code using thread API. Thus, any thread other than main thread need calling `Start()` to start running.
### 2.3 User program execution
Locate the MIPS processor allocation in the code of the `Initialize()` function and read the initialization code for this object to answer the following questions:
How are the registers of this processor initialized?
  - Create Machine emulation:
  ```c++
    machine = new Machine (debugUserProg);	// this must come first
  ``` 
  - Initialize the Machine:
  ```C++
    Machine::Machine(bool debug)
    {
        for (int i = 0; i < NumTotalRegs; i++)
            registers[i] = 0;

        mainMemory = new char[MemorySize];
        for (int i = 0; i < MemorySize; i++)
            mainMemory[i] = 0;
        ...
    }
  ``` 
* What variable is MIPS memory?
    ```c++
    char *mainMemory; // physical memory
    ``` 
* Return to the `main()` function and verify that the `StartProcess()` function is called with the name of the `../test/halt` file.
    ```c++
    if (!strcmp(*argv, "-x"))
    { // run a user program
        ASSERT(argc > 1);
        StartProcess(*(argv + 1));
        argCount = 2;
    }
    ```
Scroll through the code of the `StartProcess()` function to recognize:
* The loading of the program into memory (simulated or real?)
    -> Simulated, run under Machine
* The initialization of the registers of MIPS processor
    ```c++
    void AddrSpace::InitRegisters ()
    {
        for (int i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister (i, 0);
        machine->WriteRegister (PCReg, USER_START_ADDRESS);
        machine->WriteRegister (NextPCReg, machine->ReadRegister(PCReg) + 4);
        machine->WriteRegister (StackReg, numPages * PageSize - 16);
        DEBUG ('a', "Initializing stack register to 0x%x\n", numPages * PageSize - 16);
    }
    ```
* The execution of the MIPS processor by the `Machine::Run()` function.
    ```c++
    void Machine::Run()
    {
        Instruction the_instr;
        Instruction *instr = &the_instr;
        if(DebugIsEnabled('m'))
        printf("Starting thread \"%s\" at time %lld\n",
        currentThread->getName(), stats->totalTicks);
        interrupt->setStatus(UserMode);
        for (;;) {
            OneInstruction(instr);
            interrupt->OneTick();
            if (singleStep && (runUntilTime <= stats->totalTicks))
                Debugger();
        }
    }
    ```
Read the code of the `Machine::Run()` function and locate the function that executes a MIPS instruction.

* What is the name of the exception thrown when an addition (assembly instruction `OP_ADD`) overflows?
    ```c++
        switch (instr->opCode) {        
        case OP_ADD:
            sum = registers[instr->rs] + registers[instr->rt];
            if (!((registers[instr->rs] ^ registers[instr->rt]) & SIGN_BIT) &&
                ((registers[instr->rs] ^ sum) & SIGN_BIT)) {
                RaiseException(OverflowException, 0);
                return;
            }
            registers[instr->rd] = sum;
        break;
    ```
* Observe the end of this function: which is the register containing the program counter?
    ```c++
    // Advance program counters.
    registers[PrevPCReg] = registers[PCReg];	
    registers[PCReg] = registers[NextPCReg];
    registers[NextPCReg] = pcAfter;
    ```
## 5. Using the NachOS System
### 5.1. Kernel thread observation
    ```shell
    ./nachos -d m -x ../test/halt
    Starting thread "main" at time 10
    At PC = 0x80: JAL 4*116
    At PC = 0x84: SLL r0,r0,0
    At PC = 0x1d0: ADDIU r29,r29,-24
    At PC = 0x1d4: SW r31,16(r29)
    At PC = 0x1d8: JAL 4*114
    At PC = 0x1dc: SLL r0,r0,0
    At PC = 0x1c8: JR r0,r31
    At PC = 0x1cc: SLL r0,r0,0
    At PC = 0x98: ADDIU r2,r0,0
    At PC = 0x9c: SYSCALL
    Exception: syscall
    Machine halting!
    ```
Modify the program halt.c to introduce some calculation.
    ```c++
    #ifdef CHANGED
    // Modify the program halt.c to introduce some calculation
    int g_num1 = 10;
    int g_num2 = 20;
    #endif

    int
    main ()
    {
        #ifdef CHANGED   
        int result = g_num1 + g_num2;
        result *= g_num2 + 1;    
        #endif
        Halt ();
        /* not reached */
        #ifdef CHANGED
        return result;
        #endif
    }
    ```
* Optional question: Why is the first MIPS instruction is executed at the tenth clock tick?
    -> The main thread initialize code is as below:   
    ```c++
    void Initialize (int argc, char **argv)
    {
        ...
        currentThread = new Thread ("main");
        currentThread->setStatus (RUNNING);
        interrupt->Enable ();
        ...
    }

    void Interrupt::Enable()
    { 
        (void) SetLevel(IntOn); 
    }

    IntStatus Interrupt::SetLevel(IntStatus now)
    {
        ...        
        if ((now == IntOn) && (old == IntOff))
            OneTick();				// advance simulated time
        return old;
    }

    void Interrupt::OneTick()
    {
        ...
        if (status == SystemMode) {
            stats->totalTicks += SystemTick;
            stats->systemTicks += SystemTick;
        } else {
            stats->totalTicks += UserTick;
            stats->userTicks += UserTick;
        }
        ...
    }
    ```
During these calls, status is equal to SystemMode, and SystemTick is set to 10 clocks, that's why MIP instruction start counting at 10.
### 5.2. Kernel thread observation
* What are the compilation options of this folder?
    ```make
    DEFINES = -DTHREADS
    INCPATH = -I../threads -I../machine
    C_OFILES = $(THREAD_O)
    include ../Makefile.common
    include ../Makefile.dep
    ```
* With these compilation options, NachOS launches the ThreadTest function of `main.cc`:
  ```c++
    void ThreadTest ()
    {
        DEBUG ('t', "Entering SimpleTest\n");
        Thread *t = new Thread ("forked thread");
        t->Start (SimpleThread, (void*) 1);
        SimpleThread (0);
    }
  ```
* Let’s edit the file `threadtest.cc`, compile it, restart execution and watch carefully. Add the start of an second thread in the `ThreadTest()` function:
    ```c++
    void ThreadTest ()
    {
        DEBUG ('t', "Entering SimpleTest\n");
        Thread *t = new Thread ("forked thread");
        t->Start (SimpleThread, (void*) 1);
        #ifdef CHANGED
        Thread *t2 = new Thread ("forked thread");
        t2->Start (SimpleThread, (void*) 2);    
        #endif
        SimpleThread (0);
    }
    ```
  - Is it still working?
    ```shell
    ./nachos 
    *** thread 0 looped 0 times
    *** thread 1 looped 0 times
    *** thread 2 looped 0 times
    *** thread 0 looped 1 times
    *** thread 1 looped 1 times
    *** thread 2 looped 1 times
    *** thread 0 looped 2 times
    *** thread 1 looped 2 times
    *** thread 2 looped 2 times
    *** thread 0 looped 3 times
    *** thread 1 looped 3 times
    *** thread 2 looped 3 times
    No threads ready or runnable, and no pending interrupts.
    Assuming the program completed.
    Machine halting!
    ```
  - What does the Thread::Start() method in Nachos?
    ```c++
    void Thread::Start(VoidFunctionPtr func, void *arg)
    {
        DEBUG('t', "Starting thread \"%s\" with func = %p, arg = %d\n", name, func, arg);
        ASSERT(status == JUST_CREATED);
        StackAllocate(func, arg);
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        scheduler->ReadyToRun(this); // ReadyToRun assumes that interrupts are disabled!
        (void)interrupt->SetLevel(oldLevel);
    }
    ```
- When are created the nachos threads? (memory allocation, structure initialization)
    + Thread are created with:
        ```c++            
            Thread *t2 = new Thread ("forked thread");
            t2->Start (SimpleThread, (void*) 2);
        ``` 
    + Memory allocation and structure initialization are handled by:
        ```c++
        void Thread::StackAllocate(VoidFunctionPtr func, void *arg)
        {
            stack = (unsigned long *)AllocBoundedArray(StackSize * sizeof(unsigned long));
            valgrind_id = VALGRIND_STACK_REGISTER(stack, stack + StackSize);
            ...
            memset(&machineState, 0, sizeof(machineState));
            machineState[PCState] = (unsigned long)ThreadRoot;
            machineState[StartupPCState] = (unsigned long)SetupThreadState;
            // End of modification
            machineState[InitialPCState] = (unsigned long)func;
            machineState[InitialArgState] = (unsigned long)arg;
            machineState[WhenDonePCState] = (unsigned long)ThreadFinish;
        }
        ```
    * Now, comment the following line `currentThread->Yield();` recompile then examine what happens?
    ```c++
    void SimpleThread (void *arg)
    {
        int which = (long) arg;
        int num;
        for (num = 0; num < 10; num++)
        {
        printf ("*** thread %d looped %d times\n", which, num);
        #ifdef CHANGED
        //currentThread->Yield ();
        #endif
        }
    }
    ```
    Result:
    ```shell
    ./threads/nachos
    *** thread 0 looped 0 times
    *** thread 0 looped 1 times
    *** thread 0 looped 2 times
    *** thread 0 looped 3 times
    *** thread 0 looped 4 times
    *** thread 0 looped 5 times
    *** thread 0 looped 6 times
    *** thread 0 looped 7 times
    *** thread 0 looped 8 times
    *** thread 0 looped 9 times
    *** thread 1 looped 0 times
    *** thread 1 looped 1 times
    *** thread 1 looped 2 times
    *** thread 1 looped 3 times
    *** thread 1 looped 4 times
    *** thread 1 looped 5 times
    *** thread 1 looped 6 times
    *** thread 1 looped 7 times
    *** thread 1 looped 8 times
    *** thread 1 looped 9 times
    No threads ready or runnable, and no pending interrupts.
    Assuming the program completed.
    Machine halting!
    ```
    * What can you deduce about the preemption of the default kernel threads?
        -> Thread preemption and scheduling are processed by `currentThread->Yield();` method.
    * Restore the previously commented line. You can run NachOS by forcing a certain degree of preemption with the `-rs <n>` option.
        ```shell
        ~/code/threads$ ./nachos -rs 0
        -rs option needs a seed value        
        ```
        ```shell
        ~/code/threads$ ./nachos -rs 1
        *** thread 0 looped 0 times
        *** thread 1 looped 0 times
        *** thread 2 looped 0 times
        *** thread 0 looped 1 times
        *** thread 1 looped 1 times
        *** thread 2 looped 1 times
        No threads ready or runnable, and no pending interrupts.
        Assuming the program completed.
        Machine halting!
        ```
        ```shell
        ~/code/threads$ ./nachos -rs 7
        *** thread 0 looped 0 times
        *** thread 1 looped 0 times
        *** thread 2 looped 0 times
        *** thread 0 looped 1 times
        *** thread 1 looped 1 times
        *** thread 0 looped 2 times
        *** thread 1 looped 2 times
        No threads ready or runnable, and no pending interrupts.
        Assuming the program completed.
        Machine halting!
        ```
    * What happens? -> The sequence of executing changed, no longer 0-1-2, but may appear randomly.
    * Pair up this with the `-d + option`. How many clock ticks are there now?
    ```shell
    ~code/threads$ ./nachos -rs 7 -d +
    Scheduling interrupt handler the timer at time = 78
    interrupts: off -> on == Tick 10 == interrupts: on -> off
    ```
    * This point is quite difficult to understand. Check your intuition by commenting the line `currentThread->Yield();`
    ```shell
    ~/code/threads$ ./nachos -rs 7 -d +
    Scheduling interrupt handler the timer at time = 78
            interrupts: off -> on
    == Tick 10 ==
            interrupts: on -> off
    Time: 10, interrupts off
    Pending interrupts:
    In mapcar, about to invoke 5663dad9(576a3550)
    Interrupt handler timer, scheduled at 78
    End of pending interrupts
            interrupts: off -> on
    Entering SimpleTest
    Starting thread "forked thread" with func = 0x5663d0e6, arg = 1
            interrupts: on -> off
    Putting thread forked thread on ready list.
            interrupts: off -> on
    ...
    == Tick 79 ==
            interrupts: on -> off
    Time: 79, interrupts off
    Pending interrupts:
    In mapcar, about to invoke 5663dad9(576a3550)
    Interrupt handler timer, scheduled at 78
    End of pending interrupts
            interrupts: off -> on
    ```
    * Your conclusions? -> Context-switching/preemption are costly, multi-threading is not always improving program performance.

### 5.3. Discovering the scheduler
#### The change of explicit content
* What happens precisely when you call the `Yield()` function?
    ```c++
    void Thread::Yield()
    { 
        Thread *nextThread;
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        ASSERT(this == currentThread);
        DEBUG('t', "Yielding thread \"%s\"\n", getName());
        nextThread = scheduler->FindNextToRun();
        if (nextThread != NULL)
        {
            scheduler->ReadyToRun(this);
            scheduler->Run(nextThread);
        }
        (void)interrupt->SetLevel(oldLevel);
    }
    ```
* Inspect the code of this function in the file `code/thread/thread.cc`. When does a thread come out of this function?
-> The calling thread is rescheduled and added to the ready queue, so that it can have a chance to run in the future.

#### Scheduler Class
* Examine the methods of the Scheduler class called by the `Yield()` function. 
    ```c++
    Thread * Scheduler::FindNextToRun ()
    {
        if (halted)
        return NULL;
        return (Thread *) readyList->Remove ();
    }

    void Scheduler::ReadyToRun (Thread * thread)
    {
        DEBUG ('t', "Putting thread %s on ready list.\n", thread->getName ());
        thread->setStatus (READY);
        readyList->Append ((void *) thread);
    }

    void Scheduler::Run(Thread *nextThread)
    {
        Thread *oldThread = currentThread;

        ASSERT(interrupt->getLevel() == IntOff);

    #ifdef USER_PROGRAM // ignore until running user programs
        if (currentThread->space != NULL)
        {                                   
            currentThread->SaveUserState(); // save the user's CPU registers
            currentThread->space->SaveState();
        }
    #endif
        oldThread->CheckOverflow(); 
        currentThread = nextThread;        
        currentThread->setStatus(RUNNING); 
        DEBUG('t', "Switching from thread \"%s\" to thread \"%s\"\n", oldThread->getName(), nextThread->getName());
        SWITCH(oldThread, nextThread);
        DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());
        if (threadToBeDestroyed != NULL)
        {
            delete threadToBeDestroyed;
            threadToBeDestroyed = NULL;
        }
    #ifdef USER_PROGRAM
        if (currentThread->space != NULL)
        {                                      
            currentThread->RestoreUserState(); // to restore, do it.
            currentThread->space->RestoreState();
        }
    #endif
    }
    ```
* What are the respective roles of the `ReadyToRun()`, `FindNextToRun()`, and `Run()` functions?
    - `ReadyToRun()`: Mark a thread as ready, but not running. Put it on the ready list, and later scheduling on the CPU.
    - `FindNextToRun()`: Return the next thread to be scheduled onto the CPU.
    - `Run()`: Dispatch the CPU to nextThread.  Save the state of the old thread, and load the state of the new thread, by calling the machine dependent context switch routine.
#### In the heart of the context switch:
* In which function of the Scheduler class is the actual instruction responsible of a context switch between two processes?
    ```c++
    // Stop running oldThread and start running newThread
    void SWITCH (Thread * oldThread, Thread * newThread);
    ```
* Find the source of the corresponding low level function. What is it doing ?
    ```asm
    /* void SWITCH( thread *t1, thread *t2 )
    ** on entry, stack looks like this:
    **      8(esp)  ->              thread *t2
    **      4(esp)  ->              thread *t1
    **       (esp)  ->              return address
    ** we push the current eax on the stack so that we can use it as
    ** a pointer to t1, this decrements esp by 4, so when we use it
    ** to reference stuff on the stack, we add 4 to the offset.
    */
            .comm   _eax_save,4
            .globl  SWITCH
    SWITCH:
            movl    %eax,_eax_save          /* save the value of eax */
            movl    4(%esp),%eax            /* move pointer to t1 into eax */
            movl    %ebx,_EBX(%eax)         /* save registers */
            movl    %ecx,_ECX(%eax)
            movl    %edx,_EDX(%eax)
            movl    %esi,_ESI(%eax)
            movl    %edi,_EDI(%eax)
            movl    %ebp,_EBP(%eax)
            movl    %esp,_ESP(%eax)         /* save stack pointer */
            movl    _eax_save,%ebx          /* get the saved value of eax */
            movl    %ebx,_EAX(%eax)         /* store it */
            movl    0(%esp),%ebx            /* get return address from stack into ebx */
            movl    %ebx,_PC(%eax)          /* save it into the pc storage */
            movl    8(%esp),%eax            /* move pointer to t2 into eax */
            movl    _EAX(%eax),%ebx         /* get new value for eax into ebx */
            movl    %ebx,_eax_save          /* save it */
            movl    _EBX(%eax),%ebx         /* retore old registers */
            movl    _ECX(%eax),%ecx
            movl    _EDX(%eax),%edx
            movl    _ESI(%eax),%esi
            movl    _EDI(%eax),%edi
            movl    _EBP(%eax),%ebp
            movl    _ESP(%eax),%esp         /* restore stack pointer */
            movl    _PC(%eax),%eax          /* restore return address into eax */
            movl    %eax,0(%esp)            /* copy over the ret address on the stack */
            movl    _eax_save,%eax
            ret
    ```
### 5.4. Exercise
Change the `Yield()` method to make a context switch only once every two calls.
```c++
void Thread::Yield()
{
#ifdef CHANGED
    // Skip the context switching after 2 calls
    static int skipCounter = 0;
    const int N_SKIP = 2;
    skipCounter++;
    if ((skipCounter % N_SKIP) == 0)
        return;
#endif
    ...
}
```
Restart the nachos program in the folder threads and observe the execution (with the -d option and/or with gdb).
```shell
~code/threads$ ./nachos
*** thread 0 looped 0 times
*** thread 1 looped 0 times
*** thread 1 looped 1 times
*** thread 2 looped 0 times
*** thread 2 looped 1 times
*** thread 0 looped 1 times
```
