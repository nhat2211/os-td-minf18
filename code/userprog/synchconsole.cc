#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"

#include "synch.h"

Semaphore *SynchConsole::readAvail = new Semaphore("read avail", 0);
Semaphore *SynchConsole::writeDone = new Semaphore("write done", 0);

SynchConsole::SynchConsole(const char *in, const char *out)
{
	console = new Console(in, out, ReadAvailHandler, WriteDoneHandler, 0);

	m_readLock = new Lock("SynchConsole_ReadLock");
	m_writeLock = new Lock("SynchConsole_WriteLock");
}

SynchConsole::~SynchConsole()
{
	m_readLock->Release();
	m_writeLock->Release();

	delete m_readLock;
	delete m_writeLock;

	delete console;
	delete writeDone;
	delete readAvail;
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

void SynchConsole::ReadAvailHandler(void *arg)
{
	(void)arg;
	readAvail->V();
}

void SynchConsole::WriteDoneHandler(void *arg)
{
	(void)arg;
	writeDone->V();
}

#endif // CHANGED