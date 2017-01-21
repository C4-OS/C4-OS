#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

void _start( void *data ){
	volatile unsigned *foo = (unsigned *)0xabc;

	for (;;){
		*foo = 123;
	}
}
