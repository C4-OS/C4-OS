#include <c4rt/c4rt.h>
#include <miniforth/stubs.h>
#include <miniforth/miniforth.h>
#include <stdint.h>
#include <nameserver/nameserver.h>
#include <c4rt/interface/console.h>
#include <c4rt/interface/keyboard.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned display = 0;
//static unsigned keyboard = 0;
//static keyboard_t keyboard;
static uint32_t keyboard;

static void display_char( char c ){
	console_put_char( display, c );
}

static void debug_print( const char *s ){
	for ( ; *s; s++ ){
		display_char(*s);
	}
}

static char *read_keyboard( char *buf, unsigned n ){
	unsigned i = 0;

	debug_print( "miniforth > " );

	for ( i = 0; i < n - 1; i++ ){
		char c = 0;
retry:
		{
			keyboard_event_t ev;
			message_t msg;
			c4rt_peripheral_wait_event(&msg, keyboard);

			if (msg.type != KEYBOARD_MSG_EVENT)
				goto retry;

			keyboard_parse_event(&msg, &ev);
			/*
			keyboard_get_event(&keyboard, &ev);
			*/


			if (keyboard_event_is_modifier(&ev))
				goto retry;

			if (ev.event != KEYBOARD_EVENT_KEY_DOWN)
				goto retry;

			c = ev.character;
		}

		console_put_char(display, c);

		if (i && c == '\b') {
			i--;
			goto retry;
		}

		buf[i] = c;

		if (c == '\n') {
			break;
		}
	}

	buf[++i] = '\0';

	return buf;
}

static c4rt_file_t *cur_include = NULL;

void set_cur_include(c4rt_file_t *fp) {
	cur_include = fp;
}

static char *read_include_file(char *buf, unsigned n) {
	buf[0] = '\0';

	if (!cur_include) {
		return buf;
	}

	c4rt_fgets(buf, n, cur_include);
	if (strlen(buf) == 0) {
		cur_include = NULL;
	}

	return buf;
}

static char *read_line( char *buf, unsigned n ){
	if (cur_include) {
		return read_include_file(buf, n);

	} else {
		return read_keyboard(buf, n);
	}
}

char minift_get_char( void ){
	static char input[128];
	static bool initialized = false;
	static char *ptr;

	if (!initialized) {
		for (unsigned i = 0; i < sizeof(input); i++) { input[i] = 0; }
		// left here in case some sort of init script is added in the future
		//ptr = input;
		ptr =
			" : pstring while dup c@ 0 != begin dup c@ emit 1 + repeat ; "
			" \">> C4-OS version 0.0.1\" pstring cr "
			" \">> Type 'loadlibs' to load the default libraries.\" pstring cr "
			""
			" : loadlibs "
			"   \"/data/forth/c4.fs\" \"r\" open-file if 0 = then "
			"     include-file "
			"   end "
			" ; "
		;

		initialized = true;
	}

	while (!*ptr) {
		ptr = read_line(input, sizeof(input));
	}

	return *ptr++;
}

void minift_put_char(char c){
	console_put_char(display, c);
}

void add_c4_archives(minift_vm_t *vm);
void init_c4_allocator(minift_vm_t *vm);

static unsigned long data[0x8000];

//void _start( uintptr_t nameserver ){
int main( int argc, char *argv[], char *envp[] ){
	unsigned long calls[128];
	unsigned long params[128];
	unsigned long nameserver = getnameserv();

	// TODO: implement a better way of waiting for devices to avoid polling
	while (!display) {
		display  = nameserver_lookup(nameserver, "/dev/console");
	}

	uint32_t kbd_port = 0;
	while (!kbd_port) {
		kbd_port = nameserver_lookup(nameserver, "/dev/keyboard");
	}

	keyboard = c4_msg_create_async();
	c4rt_peripheral_connect(kbd_port, keyboard);

	minift_vm_t foo;
	minift_stack_t data_stack = {
		.start = data,
		.end   = data + sizeof(data) / sizeof(unsigned long),
		.ptr   = data,
	};

	minift_stack_t call_stack = {
		.start = calls,
		.end   = calls + 128,
		.ptr   = calls,
	};

	minift_stack_t param_stack = {
		.start = params,
		.end   = params + 128,
		.ptr   = params,
	};

	minift_init_vm(&foo, &call_stack, &data_stack, &param_stack, NULL);
	init_c4_allocator(&foo);
	add_c4_archives(&foo);
	minift_run(&foo);

	c4rt_peripheral_disconnect(keyboard);
	return 0;
}
