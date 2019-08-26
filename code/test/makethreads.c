#ifdef CHANGED

#include "syscall.h"

#define N_THREADS 10

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
}

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
	ThreadExit();
}

#endif // CHANGED
