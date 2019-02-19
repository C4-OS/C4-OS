#include <c4rt/c4rt.h>
#include <c4rt/compiler.h>
#include <c4rt/addrman.h>
#include <c4alloc/c4alloc.h>

extern char **global_env;
extern unsigned global_nameserv;
extern c4a_heap_t global_heap;
extern c4rt_vaddr_region_t *global_region;

// TODO: implement assembly stubs for some non-x86 arches which pass parameters
//       in registers instead of the stack
WEAK void _start( unsigned nameserv, char **args, char **env, unsigned magic ){
	extern int main( int argc, char **argv, char **envp );

	c4a_heap_init(&global_heap, 0xe0000000);
	// 256MB region for general-purpose memory maps
	global_region   = c4rt_vaddr_region_create(0xc0000000, 0x10000);
	global_nameserv = nameserv;

	if ( magic == C4RT_INIT_MAGIC ){
		int argc = 0;

		for ( unsigned i = 0; args[i]; i++ ){
			argc++;
			c4_debug_printf( "--- thread %u: got arg @ %p: \"%s\"\n",
							 c4_get_id(), args[i], args[i] );
		}

		for ( unsigned i = 0; env[i]; i++ ){
			c4_debug_printf( "--- thread %u: got env @ %p: \"%s\"\n",
							 c4_get_id(), env[i], env[i] );
		}

		global_env = env;
		main( argc, args, env );

	} else {
		main( 0, (char *[]){ NULL }, (char *[]){ NULL } );
	}

	c4_exit();
}
