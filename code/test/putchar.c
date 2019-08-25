#include "syscall.h"

void print(char c, int n)
{
	int i;
#if 1
	for (i = 0; i < n; i++)
	{
		PutChar(c + i);
	}
	PutChar('\n');
#endif
}

int main()
{
	char s[30];
	int n;
	print('a',4);

	// PutString("ploploploplopldsigfiodsugiodsugiofugiofdugiofdugiodfugiofdugiodu\n");
	// n = GetChar();
	// PutChar(n);
	// PutChar('\n');

	// GetString(s,30);
	// PutString(s);

	n = GetInt();
	PutInt(n);
	PutChar('\n');

	//You can test 'echo $?' in a terminal to see the '2' value
	Exit(2);
}