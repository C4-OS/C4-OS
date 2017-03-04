#ifndef _C4OS_RINGBUFFER_H
#define _C4OS_RINGBUFFER_H 1
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct c4_ringbuffer {
	unsigned read_index;
	unsigned write_index;
	size_t size;
	uint8_t data[];
} c4_ringbuf_t;

bool   c4_ringbuf_init( c4_ringbuf_t *buf, size_t size );
bool   c4_ringbuf_full( c4_ringbuf_t *buf );
bool   c4_ringbuf_empty( c4_ringbuf_t *buf );
bool   c4_ringbuf_can_write( c4_ringbuf_t *buf, size_t size );
bool   c4_ringbuf_can_read( c4_ringbuf_t *buf, size_t size );
bool   c4_ringbuf_put_byte( c4_ringbuf_t *buf, uint8_t byte );
bool   c4_ringbuf_get_byte( c4_ringbuf_t *buf, uint8_t *byte );
size_t c4_ringbuf_available( c4_ringbuf_t *buf );
size_t c4_ringbuf_write( c4_ringbuf_t *buf, void *data, size_t size );
size_t c4_ringbuf_read( c4_ringbuf_t *buf, void *data, size_t size );

#endif
