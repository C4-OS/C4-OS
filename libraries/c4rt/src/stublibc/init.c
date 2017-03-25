#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>
#include <c4rt/compiler.h>

// having a global variable here is ok, because 'env' in _start
// will be valid for the lifetime of the program
static char **global_env = NULL;
static unsigned global_nameserv = 0;

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
