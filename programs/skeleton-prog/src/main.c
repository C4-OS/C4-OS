#include <c4rt/c4rt.h>
#include <stdint.h>
#include <stdbool.h>

int main(int argc, char *argv[]){
	c4_debug_printf( "--- skeleton-prog: hello, world! thread %u\n",
	                 c4_get_id());

	while ( true ){
		message_t msg;

		c4_msg_recieve( &msg, 0 );
	}

	c4_exit();
}
