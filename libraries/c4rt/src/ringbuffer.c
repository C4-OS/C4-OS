#include <c4rt/ringbuffer.h>
#include <c4rt/c4rt.h>

static inline unsigned mask( c4_ringbuf_t *buf, unsigned value ){
	return value % buf->size;
}

static inline unsigned increment( c4_ringbuf_t *buf, unsigned value ){
	return mask( buf, value + 1 );
}

bool c4_ringbuf_init( c4_ringbuf_t *buf, size_t size ){
	if ( size > sizeof(c4_ringbuf_t) + 1 ){
		buf->read_index  = 0;
		buf->write_index = 0;
		buf->size = size - sizeof(c4_ringbuf_t);

		return true;
	}

	return false;
}

bool c4_ringbuf_full( c4_ringbuf_t *buf ){
	return increment(buf, buf->write_index) == buf->read_index;
}

bool c4_ringbuf_empty( c4_ringbuf_t *buf ){
	return buf->read_index == buf->write_index;
}

bool c4_ringbuf_can_write( c4_ringbuf_t *buf, size_t size ){
	return size < c4_ringbuf_available(buf);
}

bool c4_ringbuf_can_read( c4_ringbuf_t *buf, size_t size ){
	return size < buf->size - c4_ringbuf_available(buf);
}

bool c4_ringbuf_put_byte( c4_ringbuf_t *buf, uint8_t byte ){
	if ( !c4_ringbuf_full( buf )){
		buf->data[buf->write_index] = byte;
		buf->write_index = increment(buf,  buf->write_index);
		return true;
	}

	return false;
}

bool c4_ringbuf_get_byte( c4_ringbuf_t *buf, uint8_t *byte ){
	if ( !c4_ringbuf_empty( buf )){
		*byte = buf->data[buf->read_index];
		buf->read_index = increment(buf, buf->read_index);
		return true;
	}

	return false;
}

size_t c4_ringbuf_available( c4_ringbuf_t *buf ){
	return buf->size - mask(buf, buf->write_index - buf->read_index);
}

size_t c4_ringbuf_write( c4_ringbuf_t *buf, void *data, size_t size ){
	size_t wrote = 0;
	uint8_t *u8_data = data;

	for ( size_t i = 0; i < size; i++ ){
		if ( !c4_ringbuf_put_byte( buf, u8_data[i] ))
			break;

		wrote++;
	}

	return wrote;
}

size_t c4_ringbuf_read( c4_ringbuf_t *buf, void *data, size_t size ){
	size_t read = 0;
	uint8_t *u8_data = data;

	for ( size_t i = 0; i < size; i++ ){
		if ( !c4_ringbuf_get_byte( buf, u8_data + i ))
			break;

		read++;
	}

	return read;
}
