/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

#ifdef CHANGED
// Modify the program halt.c to introduce some calculation
int g_num1 = 10;
int g_num2 = 20;
#endif

int
main ()
{
    #ifdef CHANGED
    // Modify the program halt.c to introduce some calculation
    int result = g_num1 + g_num2;
    result *= g_num2 + 1;    
    #endif

    Halt ();

    /* not reached */
    #ifdef CHANGED
    return result;
    #endif
}
