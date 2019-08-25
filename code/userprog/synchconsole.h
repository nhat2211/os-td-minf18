#ifdef CHANGED

#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"

class SynchConsole:dontcopythis
{
public:
	SynchConsole(const char *readFile, const char *writeFile); // initialize the hardware console device
	~SynchConsole();                                           // clean up console emulation

	void SynchPutChar(int ch);                                 // Unix putchar(3S)
	int SynchGetChar();                                        // Unix getchar(3S)
	void SynchPutString(const char *s);                        // Unix fputs(3S)
	void SynchGetString(char *s, int n);                       // Unix fgets(3S)
	void PutChar(const char s[]);
	void GetChar(char *s);
	//(5.2) Write a procedure similar to strcpy:
	int copyStringFromMachine(int from , char * to , unsigned size);

private:
	static void ReadAvailHandler(void *arg);
	static void WriteDoneHandler(void *arg);

	static Semaphore *readAvail;
	static Semaphore *writeDone;

	Console *console;
};

#endif // SYNCHCONSOLE_H

#endif // CHANGED