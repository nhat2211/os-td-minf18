#ifdef CHANGED

#ifdef USER_PROGRAM

#include "userthread.h"
#include "machine.h"
#include "system.h"
#include "synch.h"
#include "bitmap.h"

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
        int f_exit = plop[2];
        int idx = plop[3];

        DEBUG('x', "f: %d\n", f);
        DEBUG('x', "arg: %d\n", arg);
        DEBUG('x', "f_exit: %d\n", f_exit);
        DEBUG('x', "idx: %d\n", idx);

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
        DEBUG('x', "running now\n");
        machine->Run();

        delete plop; // free memory
    }
}

int do_ThreadCreate(int f, int arg, int f_exit)
{
    DEBUG('x', "do_ThreadCreate: %d, %d\n", f, arg);

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

            DEBUG('x', "newThread->Start: %d, %d, %d, %d, %d\n", plop[0], plop[1], plop[2], plop[3], g_threads[idx]);
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
    DEBUG('x', "do_ThreadExit()\n");

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

#endif // USER_PROGRAM
#endif // CHANGED
