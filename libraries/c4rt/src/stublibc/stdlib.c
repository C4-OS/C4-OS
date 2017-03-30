#include <c4rt/stublibc.h>
#include <c4alloc/c4alloc.h>

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
void realloc( void *ptr, size_t size );
