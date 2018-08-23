#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]){
	volatile unsigned *foo = (unsigned *)0xabc;

	for (;;){
		*foo = 123;
	}
}
