# MINF18
TD OS Reporting
Devoir 1
Members:
Vo Hung Son
Tran Quang Nhat

Concurrency and synchronization

## 1. What is the goal?
```c
    #include "syscall.h"

    void print(char c, int n)
    {
        int i;
        #if 0
            for (i = 0; i < n; i++)
            {
                PutChar(c + i);
            }
            PutChar('\n');
        #endif
    }

    int main()
    {
        print('a',4);
        Halt();
    }
```
* What is the reasonably expected output considering #if 0 transformed to #if 1?
- The code above put the sequence of characters, it puts the n characters with the sequence characters like a, b, c... when #if 0. If print in the console, we will get the result like: abcd.

## 2. Asynchronous Inputs Outputs
Explain why it is a mistake to try:
* To read a character before being warned that a character is available.
    - because Write "ch" to the console display, and return immediately. "writeDone" is called when the I/O completes.
  
* To try to write before being warned that the previous writing is complete.
    - because the previous writing must be complete then "writeDone" is called when the I/O completes so to try to write before the error will be occur.

### 2.1 Examine the `userprog/progtest.cc` program.
```c
void ConsoleTest (const char *in, const char *out)
{
    char ch;
    readAvail = new Semaphore ("read avail", 0);
    writeDone = new Semaphore ("write done", 0);
    console = new Console (in, out, ReadAvailHandler, WriteDoneHandler, 0);

    for (;;)
    {
        readAvail->P ();	// wait for character to arrive
        ch = console->GetChar ();

        console->PutChar (ch);    // echo it!
        writeDone->P ();  // wait for write to finish
        if (ch == 'q') {
            printf ("Nothing more, bye!\n");
            break;	
        }
    }

    delete console;
    delete readAvail;
    delete writeDone;
}
```

### 2.2 Modify `userprog/progtest.cc` to display "Goodbye" to end of file (EOF), in addition to the character ’q’.
```c
void ConsoleTest (const char *in, const char *out)
{
    char ch;
    readAvail = new Semaphore ("read avail", 0);
    writeDone = new Semaphore ("write done", 0);
    console = new Console (in, out, ReadAvailHandler, WriteDoneHandler, 0);

    for (;;)
    {
        readAvail->P ();	// wait for character to arrive
        ch = console->GetChar ();

        console->PutChar (ch);    // echo it!
        writeDone->P ();  // wait for write to finish

        if (ch == 'q') {
            printf ("Nothing more, bye!\n");
        #ifdef CHANGED
            printf ("Goodbye!\n");
        #endif
            break;		// if q, quit
        }
    }
    
    delete console;
    delete readAvail;
    delete writeDone;
}
```

### 2.3 Modify `userprog/progtest.cc` to write “<x>” instead of “x” in the loop body
```c
void ConsoleTest (const char *in, const char *out)
{
    char ch;
    readAvail = new Semaphore ("read avail", 0);
    writeDone = new Semaphore ("write done", 0);
    console = new Console (in, out, ReadAvailHandler, WriteDoneHandler, 0);

    for (;;)
    {
        readAvail->P ();	// wait for character to arrive
        ch = console->GetChar ();

    #ifdef CHANGED
        console->PutChar ('<');
        writeDone->P ();
    #endif
        console->PutChar (ch);    // echo it!
        writeDone->P ();  
    #ifdef CHANGED
        console->PutChar ('>');
        writeDone->P ();
    #endif
        if (ch == 'q') {
            printf ("Nothing more, bye!\n");
        #ifdef CHANGED
            printf ("Goodbye!\n");
        #endif
            break;		// if q, quit
        }
    }
    delete console;
    delete readAvail;
    delete writeDone;
}
```

### 2.4 Verify that this also works with an input file and an output file
```c
void ConsoleTest (const char *in, const char *out)
{
    char ch;
    readAvail = new Semaphore ("read avail", 0);
    writeDone = new Semaphore ("write done", 0);
    console = new Console (in, out, ReadAvailHandler, WriteDoneHandler, 0);
    for (;;)
    {
        readAvail->P ();	// wait for character to arrive
        ch = console->GetChar ();
    #ifdef CHANGED
        if(ch != '\n')
        {
            console->PutChar ('<');
            writeDone->P ();
        }        
    #endif
        console->PutChar (ch);    // echo it!
        writeDone->P ();  // wait for write to finish
    #ifdef CHANGED
        if(ch != '\n')
        {
            console->PutChar ('>');
            writeDone->P ();
        }
    #endif
        if (ch == 'q') {
            printf ("Nothing more, bye!\n");
        #ifdef CHANGED
            printf ("Goodbye!\n");
        #endif
            break;		// if q, quit
        }
    }
    delete console;
    delete readAvail;
    delete writeDone;
}
```

## 3. Synchronous Inputs Outputs
### 3.1 `synchconsole.h`
```c++
class SynchConsole : dontcopythis
{
public:
	SynchConsole(const char *readFile, const char *writeFile); // initialize the hardware console device
	~SynchConsole();										   // clean up console emulation

	void SynchPutChar(int ch);			 // Unix putchar(3S)
	int SynchGetChar();					 // Unix getchar(3S)
	void SynchPutString(const char *s);  // Unix fputs(3S)
	void SynchGetString(char *s, int n); // Unix fgets(3S)

private:
	static void ReadAvailHandler(void *arg);
	static void WriteDoneHandler(void *arg);

	static Semaphore *readAvail;
	static Semaphore *writeDone;

	Console *console;
};
```

### 3.2 `synchconsole.cc`
```c++
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
void SynchConsole::ReadAvailHandler(void *arg)
{
	(void)arg; readAvail->V();
}
void SynchConsole::WriteDoneHandler(void *arg)
{
	(void)arg; writeDone->V();
}
oid SynchConsole::SynchPutChar(int ch)
{
	console->PutChar(ch); writeDone->P();
}
int SynchConsole::SynchGetChar()
{
	readAvail->P(); return console->GetChar();
}
void SynchConsole::SynchPutString(const char s[])
{
	int i = 0;
	while (s[i] != '\0')
	{
		console->PutChar(s[i++]); writeDone->P();
	}
}
void SynchConsole::SynchGetString(char *s, int n)
{
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
}
```

### 3.3 `Makefile.common`
```makefile
USERPROG_O	:=	addrspace.o bitmap.o exception.o progtest.o console.o synchconsole.o \
			machine.o mipssim.o translate.o
```

### 3.4 `threads/main.cc`
```c++
int main(int argc, char **argv)
{
    ...
    if (argc > 1 && !strcmp(argv[1], "-h")) // print help
	{
        printf(
			"Usage: nachos -d <debugflags> -rs <random seed #> -z -h\n"
            ...
        #ifdef CHANGED
			"-sc tests the synchconsole\n"
        #endif
            ...
    }

    ...

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount)
	{
        ...
    #ifdef CHANGED
		else if (!strcmp(*argv, "-sc"))
		{
			if (argc == 1) {
                SynchConsoleTest(NULL, NULL);
            }
			else
			{
				ASSERT(argc > 2);
				SynchConsoleTest(*(argv + 1), *(argv + 2));
				argCount = 3;
			}
		}
    #endif
        ...
    }

```

### 3.5 `progtest.cc`
```c++
#ifdef CHANGED
void SynchConsoleTest (const char *in, const char *out)
{
  char ch;
  SynchConsole *test_synchconsole = new SynchConsole(in, out);

  while ((ch = test_synchconsole->SynchGetChar()) != EOF)
  {    
    test_synchconsole->SynchPutChar(ch);    
  }
  fprintf(stderr, "EOF detected in SynchConsole!\n");
}
```

### 3.6 Display '<' and '>'
```c++
#ifdef CHANGED
void SynchConsoleTest (const char *in, const char *out)
{
  char ch;
  SynchConsole *test_synchconsole = new SynchConsole(in, out);

  while ((ch = test_synchconsole->SynchGetChar()) != EOF)
  {
    if (ch != '\n')
      test_synchconsole->SynchPutChar('<');

    test_synchconsole->SynchPutChar(ch);    

    if (ch != '\n')
      test_synchconsole->SynchPutChar('>');
  }
  fprintf(stderr, "EOF detected in SynchConsole!\n");
}
```

## 4. PutChar System Call
### 4.1 Add a “#define SC_PutChar ...” system call
```c++
#ifdef CHANGED
    #define SC_PutChar   11
#endif
```

### 4.2 The PutChar(char c) function
```c++
#ifdef CHANGED
    void PutChar(char c);
#endif
```

### 4.3 The assembly code of PutChar()
```c++
	.globl PutChar
	.ent	PutChar
PutChar:
	addiu $2,$0,SC_PutChar
	syscall
	j	$31
	.end PutChar
```

### 4.4 - 4.5 Set up the handler which is activated by the syscall interrupt
```c++
void ExceptionHandler (ExceptionType which)
{
    int type = machine->ReadRegister (2);
    switch (which)
    {
        ...
    #ifdef CHANGED
		case SC_PutChar:
		  {
			DEBUG('s',"PutChar\n");
			int ch = machine->ReadRegister(4);
			synchconsole->SynchPutChar(ch);
			break;
		  }
    #endif
        ...
    }
}
```

### 4.6 Edit the `threads/system.cc`
```c++
#ifdef USER_PROGRAM
#ifdef CHANGED
    SynchConsole *synchconsole;
#endif
#endif
...

void Initialize (int argc, char **argv)
{
    ...
#ifdef USER_PROGRAM
    machine = new Machine (debugUserProg);
#ifdef CHANGED
    synchconsole = new SynchConsole(NULL,NULL);
#endif 
#endif
    ...
}

void Cleanup ()
{
    ...
#ifdef USER_PROGRAM
#ifdef CHANGED
    delete synchconsole;
    synchconsole = NULL;
#endif
    delete machine;
    machine = NULL;
#endif
#endif
    ...
}
```

### 4.7 Edit the `threads/system.h`
```c++
#ifdef USER_PROGRAM
    #include "machine.h"
    extern Machine *machine;
#ifdef CHANGED
    #include "synchconsole.h"
    extern SynchConsole *synchconsole;
#endif
#endif
```

## 5. From characters to strings
### 5.1 `SynchPutString()`
```c++
void SynchConsole::SynchPutString(const char s[])
{
	int i=0;
	while( s[i] != '\0' )
	{
		console->PutChar( s[i++] );
		writeDone->P ();
    }
}
```

### 5.2 `copyStringFromMachine()`
```c++
/** in system.cc */

Machine *machine;

#ifdef CHANGED
int copyStringFromMachine(int from, char *to, unsigned size)
{
    int tmp;
    bool ok = true;
    unsigned pos = 0;
    while ( ok && pos < size-1 )
    {
        ok = machine->ReadMem(from++, 1, &tmp);
        if ( ok && (char)tmp != '\0' )
            to[pos] = (char)tmp;
        else
            ok = false;
        pos++;
    }
    to[pos] = '\0';
    return pos;
}
#endif
```
* Beware of the `int *value` argument of ReadMem: why cannot we just pass a pointer pointing inside the buffer to?
    - Because if we pass the buffer pointer, then the whole buffer will be overwritten and will return unexpected result.

### 5.3 `PutString()` system call
```c++
/** syscall.h */
#ifdef CHANGED
#define SC_PutChar   11
#define SC_PutString 12
...
#endif

#ifdef CHANGED
void PutChar(char c);
void PutString(char s[]);
#endif
```

```c++
/** start.S */

	.globl PutString
	.ent	PutString
PutString:
	addiu $2,$0,SC_PutString
	syscall
	j	$31
	.end PutString
```

```c++
/** system.h */

#define MAX_STRING_SIZE	15
```

```c++
/** exception.cc */

void ExceptionHandler (ExceptionType which)
{
    ...
    switch (which)
    {
        ...
        case SC_PutString:
        {            
            char st[MAX_STRING_SIZE];
            int reg = machine->ReadRegister(4);
            copyStringFromMachine(reg, st, MAX_STRING_SIZE);
            synchconsole->SynchPutString(st);
            break;
        }
    }
    ...
}
```
* Why would not it be reasonable to allocate, to a buffer, the same size than the MIPS string?
    - Because we would not know the size of the MIPS string in advance, before we actually call ReadMem and get the string terminal character '/0'.

### 5.4 Examples
```c++
int main()
{
	PutString("1234567890abcdefg\n");
}
```

```shell
phi@phi-pc:~/nacho/code$ ./userprog/nachos -x ./test/putchar
1234567890abcd
```
* The MAX_STRING_SIZE set to 15, thus it only put 15 first characters of the string to the console.
    - Possible fixes:
        + Increase the MAX_STRING_SIZE to something larger (256, 512, ...) => Costly, and waste of memory for short string, not recommended.
        + Change the `copyStringFromMachine()` method, to read the string to a buffer until getting the `'/0'` character:
            ```c++
            /** system.h */

            extern int copyStringFromMachine(int from, char *&to, unsigned size);
            ``` 
            ```c++
            /** system.cc */

            int copyStringFromMachine(int from, char *&to, unsigned size)
            {
                int tmp;
                to = new char[size];
                bool ok = true;
                unsigned pos = 0;
                while (ok)
                {
                    ok = machine->ReadMem(from++, 1, &tmp);
                    if (ok && (char)tmp != '\0')
                    {
                        if (pos >= size - 1)
                        {                
                            char *tmp_str = new char [size * 2 + 1];
                            for (int i = 0; i < size; i++)
                            {
                                tmp_str[i] = to[i];
                            }
                            size = size * 2;
                            delete to; // 'to' is dynamic allocated
                            to = tmp_str;
                        }
                        to[pos] = (char)tmp;
                    }
                    else
                        ok = false;
                    pos++;
                }
                to[pos] = '\0';   
                return pos;
            }
            ``` 
            ```c++
            /** exception.cc */

            void ExceptionHandler (ExceptionType which)
            {
                ...
                switch (which)
                {
                    ...
                    case SC_PutString:
                    {            
                        char *st; //[MAX_STRING_SIZE];
                        int reg = machine->ReadRegister(4);
                        copyStringFromMachine(reg, st, MAX_STRING_SIZE);
                        synchconsole->SynchPutString(st);
                        break;
                    }
                    ...
                }
                ...
            }
            ```
            ```shell
            phi@phi-pc:~/nacho/code$ ./userprog/nachos -x ./test/putchar
            1234567890abcdefg
            ```
## 6. But how to stop?
* What happens if you remove the call to `Halt()` at the end of the main function of `putchar.c`?
    - In my PC, it print the input to console, then quit without printing machine's statistic. However, nothing wrong happened (exception nor error, ...)

* What to change to avoid this error?
    - Option 1: Put back `Halt()` to make it quit properly with statistic printed and goodbye message.
    - Option 2: Add the `Exit(0)` syscall to make it quit properly, with return code from emulated program.

* How to take into account the return value return n of the main function if this is declared as a integer value?
    - We can use `Exit()` syscall to return the error code to the Linux program.

* Look for test “who” call the main function.
    ```c++
    /* -------------------------------------------------------------
    * __start
    *	Initialize running a C program, by calling "main". 
    *
    * 	NOTE: This has to be first, so that it gets loaded at location 0.
    *	The Nachos kernel always starts a program by jumping to location 0.
    * -------------------------------------------------------------
    */

        .org USER_START_ADDRESS

        .globl __start
        .ent	__start
    __start:
        jal	main
        move	$4,$0		
        jal	Exit	 /* if we return from main, exit(0) */
        .end __start
    ```

## 7. Reading Functions
### 7.1 Complete the `GetChar` system call.
    ```c++
    /** syscall.h */
    #ifdef CHANGED
    #define SC_PutChar   11
    #define SC_PutString 12
    #define SC_GetChar   13
    ...
    #endif

    #ifdef CHANGED
    void PutChar(char c);
    void PutString(char s[]);
    int GetChar();
    #endif
    ```

    ```c++
    /** start.S */

        .globl GetChar
        .ent	GetChar
    GetChar:
        addiu $2,$0,SC_GetChar
        syscall
        j	$31
        .end GetChar
    ```
 
    ```c++
    /** exception.cc */

    void ExceptionHandler (ExceptionType which)
    {
        ...
        switch (which)
        {
            ...
            case SC_GetChar:
            {
                DEBUG('s',"GetChar\n");
                int readValue = synchconsole->SynchGetChar();
                machine->WriteRegister(2,(char)readValue);
                break;
            }
            ...
        }
        ...
    }
    ```
* What do you do in case of end of file?
    - EOF is returned when there is no character input in the console. In this case, just check the return character before calling function to it:
        ```c++
        n = GetChar();

       	if(n != '\n') // '\n' mean nothing entered but 'ENTER'
        {
            PutChar(n);
            PutChar('\n');
        }
        ```


### 7.2 Complete the `SynchGetString` method of the `SynchConsole` class.
```c++
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
```

### 7.3 Complete the `GetString` system.
```c++
/** syscall.h */
#ifdef CHANGED
#define SC_PutChar   11
#define SC_PutString 12
#define SC_GetChar   13
#define SC_GetString 14
...
#endif

#ifdef CHANGED
void PutChar(char c);
void PutString(char s[]);
int GetChar();
void GetString(char *s, int n);
#endif
```

```c++
/** start.S */

    .globl GetString
    .ent	GetString
GetString:
    addiu $2,$0,SC_GetString
    syscall
    j	$31
    .end GetString
```

```c++
/** exception.cc */

void ExceptionHandler (ExceptionType which)
{
    ...
    switch (which)
    {
        ...
        case SC_GetString:
        {
            DEBUG('s',"GetString\n");
            char st[MAX_STRING_SIZE];
            int reg = machine->ReadRegister(4);
            synchconsole->SynchGetString(st, MAX_STRING_SIZE);
            copyStringToMachine(st, reg, MAX_STRING_SIZE);
            break;
        }
        ...
    }
    ...
}
```
* What would happen if multiple threads simultaneously called this function?
    - The `GetString` system call is handled with Semaphore, thus it would allow only one thread at a time. Thus, the concurrency is no problem with this implementation

### 7.4 (Bonus) Complete the `PutInt` and `GetInt` system call.
```c++
/** syscall.h */
#ifdef CHANGED
#define SC_PutChar   11
#define SC_PutString 12
#define SC_GetChar   13
#define SC_GetString 14
#define SC_PutInt    15
#define SC_GetInt    16
...
#endif

#ifdef CHANGED
void PutChar(char c);
void PutString(char s[]);
int GetChar();
void GetString(char *s, int n);
void PutInt(int n);
int GetInt();
#endif
```

```c++
/** start.S */

    .globl PutInt
    .ent	PutInt
PutInt:
    addiu $2,$0,SC_PutInt
    syscall
    j	$31
    .end PutInt

    .globl GetInt
    .ent	GetInt
GetInt:
    addiu $2,$0,SC_GetInt
    syscall
    j	$31
    .end GetInt
```

```c++
/** exception.cc */

void ExceptionHandler (ExceptionType which)
{
    ...
    switch (which)
    {
        ...
        case SC_PutInt:
        {
            DEBUG('s',"PutInt\n");
            char st[MAX_STRING_SIZE];
            int val = machine->ReadRegister(4);
            snprintf(st, MAX_STRING_SIZE, "%d", val);
            synchconsole->SynchPutString(st);
            break;
        }
        case SC_GetInt:
        {
            DEBUG('s',"GetInt\n");
            char st[MAX_STRING_SIZE];
            synchconsole->SynchGetString(st, MAX_STRING_SIZE);
            int n;
            sscanf(st, "%d", &n);
            machine->WriteRegister(2,n);
            break;
        }
        ...
    }
    ...
}
```
## 8. Bonus: a printf
* Why is it a bad idea to set up a Printf system call?
    - System call is a costly operation, thus it should be extremely fast and efficient. Printf put sequence of character to console, which is an I/O bound operation which cost a lot of time to finish. So, in general it's not a good idea to implement Printf as a system call.

