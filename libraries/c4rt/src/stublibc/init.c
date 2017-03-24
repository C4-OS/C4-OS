#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>
#include <c4rt/compiler.h>

WEAK int main( int argc, char **argv, char **envp ){
	unsigned id = c4_get_id();

	c4_debug_printf(
		"--- thread %u: it seems you have compiled this program with neither\n"
		"--- thread %u: a _start() or main() function, can't continue...\n"
		"--- thread %u: (this function is the default main stub in %s)\n",
		id, id, id, __FILE__ );

	c4_exit();
	return 0;
}

// TODO: implement assembly stubs for some non-x86 arches which pass parameters
//       in registers instead of the stack
WEAK void _start( unsigned nameserv, char **args, char **env, unsigned magic ){
	extern int main( int argc, char **argv, char **envp );

	if ( magic == C4RT_INIT_MAGIC ){
		for ( unsigned i = 0; args[i]; i++ ){
			c4_debug_printf( "--- thread %u: got arg @ %p: \"%s\"\n",
							 c4_get_id(), args[i], args[i] );
		}

		for ( unsigned i = 0; env[i]; i++ ){
			c4_debug_printf( "--- thread %u: got env @ %p: \"%s\"\n",
							 c4_get_id(), env[i], env[i] );
		}

		main( 0, args, env );

	} else {
		main( 0, (char *[]){ NULL }, (char *[]){ NULL } );
	}

	c4_exit();
}
