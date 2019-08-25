#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"

Semaphore* SynchConsole::readAvail = new Semaphore("read avail", 0);
Semaphore* SynchConsole::writeDone = new Semaphore("write done", 0);

SynchConsole::SynchConsole(const char *in, const char *out)
{
	console = new Console(in, out, ReadAvailHandler, WriteDoneHandler, 0);
}

SynchConsole::~SynchConsole()
{
	delete console;
	delete writeDone;
	delete readAvail;
}

void SynchConsole::SynchPutChar(int ch)
{
	console->PutChar (ch);
    writeDone->P ();
}

int SynchConsole::SynchGetChar()
{
	readAvail->P ();
	return console->GetChar ();
}

void SynchConsole::SynchPutString(const char s[])
{
	int i=0;
	while( s[i] != '\0' )
	{
		console->PutChar( s[i++] );
		writeDone->P ();
    }
}

void SynchConsole::SynchGetString(char *s, int n)
{
	int c;
	while (n>1)
	{
		readAvail->P ();
		c = console->GetChar ();
		*s++ = (char)c;
		if ( (char)c == '\n' )
			n=0;
		else n--;
	}
	*s = '\0';
}

void SynchConsole::PutChar(const char s[])
{
	int i=0;
	while( s[i] != '\0' )
	{
		console->PutChar( s[i++] );
		writeDone->P ();
    }
}

void SynchConsole::GetChar(char *s, int n)
{
	int c;
	while (n>1)
	{
		readAvail->P ();
		c = console->GetChar ();
		*s++ = (char)c;
		if ( (char)c == '\n' )
			n=0;
		else n--;
	}
	*s = '\0';
}

void SynchConsole::ReadAvailHandler(void *arg)
{
	(void) arg;
	readAvail->V();
}

void SynchConsole::WriteDoneHandler(void *arg)
{
	(void) arg;
	writeDone->V();
}
//(5.2) Write a procedure similar to strcpy:
void SynchConsole::copyStringFromMachine (int from, char *to, unsigned size) {
        unsigned i;
        for(i=0;i<size && machine->mainMemory[from+i] !='\0';i++) {
                to[i] = machine->mainMemory[from+i];
        }
        to[i] = '\0';
}

#endif // CHANGED