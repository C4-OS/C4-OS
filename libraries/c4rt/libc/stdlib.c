#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <c4alloc/c4alloc.h>
#include <c4rt/addrman.h>
#include <c4rt/elf.h>
#include <c4rt/c4rt.h>
#include <c4rt/prng.h>

WEAK void *malloc( size_t size ){
	return c4a_alloc( getc4heap(), size );
}

WEAK void free( void *ptr ){
	if ( ptr ){
		c4a_free( getc4heap(), ptr );
	}
}

void *calloc( size_t members, size_t size ){
	void *ret = malloc( members * size );

	if ( ret ){
		memset( ret, 0, members * size );
	}

	return ret;
}

// TODO: implement realloc()
void *realloc( void *ptr, size_t size );

int rand(void) {
	return (int)c4rt_prng_u32();
}

void srand(unsigned int seed) {
	c4rt_prng_seed(seed);
}

int abs(int j) {
	return (j < 0)? -j : j;
}

c4_process_t spawn(const char *name, const char *argv[], const char *envp[]){
	// XXX: fixed-size buffer to store elfs from files, this will limit the size
	//      of executables which can be loaded, and wastes memory
	//
	// TODO: dynamically allocate/free file buffers, or read from elf directly
	//       into memory for the new process (which will require being able to
	//       set the byte location of a file stream)
	static uint8_t progbuf[PAGE_SIZE * 128] ALIGN_TO(PAGE_SIZE);
	c4_process_t ret;

	FILE *fp = fopen(name, "r");

	C4_ASSERT(fp != NULL);
	if (!fp) {
		memset(&ret, 0, sizeof(c4_process_t));
		return ret;
	}

	size_t i = 0;
	int nread = 0;

	while ((nread = fread(progbuf + i, PAGE_SIZE, 1, fp)) > 0) {
		i += nread;
	}

	fclose(fp);

	if (!elf_is_valid((Elf32_Ehdr*)progbuf)) {
		memset(&ret, 0, sizeof(c4_process_t));
		return ret;
	};

	ret = elf_load((void*)progbuf, argv, envp);

	return ret;
}
