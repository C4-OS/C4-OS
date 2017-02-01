#include <c4rt/c4rt.h>
#include <c4/thread.h>
#include <stdint.h>

enum {
	BENCHMARK_ITERS = 10000,
	BENCHMARK_PING = 0xbad,
	BENCHMARK_PONG,
	BENCHMARK_EXIT,
};

static inline uint64_t get_timestamp( void ){
	register unsigned a, d;

	asm volatile ( "rdtsc" : "=a"(a), "=d"(d));

	return ((uint64_t)d << 32) | a;
}

static void reciever( void ){
	message_t msg;
	bool running = true;

	while ( running ){
		c4_msg_recieve( &msg, 0 );
		uint64_t stamp = get_timestamp();

		switch ( msg.type ){
			case BENCHMARK_PING:
				msg.type = BENCHMARK_PONG;
				msg.data[0] = stamp >> 32;
				msg.data[1] = stamp &  0xffffffff;
				c4_msg_send( &msg, msg.sender );
				break;

			case BENCHMARK_EXIT:
				running = false;
				c4_exit();
				break;

			default:
				break;
		}
	}
}

static void do_benchmark( unsigned reciever ){
	uint64_t benchmark_start = get_timestamp();

	unsigned round_sum = 0;
	unsigned oneway_sum = 0;

	for ( unsigned i = 0; i < BENCHMARK_ITERS; i++ ){
		uint64_t stamp = get_timestamp();
		message_t msg  = { .type = BENCHMARK_PING, };

		c4_msg_send( &msg, reciever );
		c4_msg_recieve( &msg, reciever );

		uint64_t end_stamp = get_timestamp();
		uint64_t mid_stamp = ((uint64_t)msg.data[0] << 32) | msg.data[1];

		unsigned round  = end_stamp - stamp;
		unsigned oneway = mid_stamp - stamp;

		round_sum  += round;
		oneway_sum += oneway;
	}

	uint64_t benchmark_end = get_timestamp();

	unsigned round_avg  = round_sum  / BENCHMARK_ITERS;
	unsigned oneway_avg = oneway_sum / BENCHMARK_ITERS;
	unsigned benchmark_len = benchmark_end - benchmark_start;

	c4_debug_printf( "--- ipcbench:   round trip: %u cycles\n", round_avg );
	c4_debug_printf( "--- ipcbench:   one way:    %u cycles\n", oneway_avg );
	c4_debug_printf( "--- ipcbench:   stats:      %u total cycles, %u messages\n",
					 benchmark_len, BENCHMARK_ITERS );
}

static inline void stop_benchmark( unsigned reciever ){
	message_t msg = { .type = BENCHMARK_EXIT, };

	c4_msg_send( &msg, reciever );
}

static unsigned long inter_reciever_stack[128];
static unsigned long intra_reciever_stack[128];

void _start( unsigned long nameserver ){
	unsigned inter_id = c4_create_thread( reciever,
	                                      inter_reciever_stack + 128, 
	                                      THREAD_CREATE_FLAG_CLONE );

	unsigned intra_id = c4_create_thread( reciever,
	                                      intra_reciever_stack + 128, 
	                                      0 );

	c4_continue_thread( inter_id );
	c4_continue_thread( intra_id );

	c4_debug_printf( "--- ipcbench: started recievers\n" );
	c4_debug_printf( "--- ipcbench: testing ipc between address spaces:\n" );
	do_benchmark( inter_id );

	c4_debug_printf( "--- ipcbench: testing ipc in same address space:\n" );
	do_benchmark( intra_id );

	stop_benchmark( inter_id );
	stop_benchmark( intra_id );

	c4_debug_printf( "--- ipcbench: done, exiting\n" );

	c4_exit();
}
