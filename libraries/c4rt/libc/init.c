#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>
#include <c4rt/compiler.h>
#include <c4rt/addrman.h>
#include <c4alloc/c4alloc.h>

// having a global variable here is ok, because 'env' in _start
// will be valid for the lifetime of the program
char **global_env = NULL;
unsigned global_nameserv = 0;
c4a_heap_t global_heap;
c4rt_vaddr_region_t *global_region;

WEAK char *getenv( const char *name ){
	if ( !global_env )
		return NULL;

	for ( size_t n = 0; global_env[n]; n++ ){
		size_t len = strcspn( global_env[n], "=" );

		if ( strlen(name) != len ){
			return NULL;
		}

		if ( strncmp( name, global_env[n], len ) == 0 ){
			return strchr( global_env[n], '=' ) + 1;
		}
	}

	return NULL;
}

WEAK unsigned getnameserv( void ){
	return global_nameserv;
}

WEAK c4a_heap_t *getc4heap( void ){
	return &global_heap;
}

WEAK c4rt_vaddr_region_t *get_genregion(void){
	return global_region;
}
