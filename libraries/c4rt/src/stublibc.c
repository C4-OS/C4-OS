#include <c4rt/stublibc.h>
#include <stdint.h>

WEAK void *memcpy( void *dest, const void *src, size_t n ){
	uint8_t *d = dest;
	const uint8_t *s = src;

	for ( size_t i = 0; i < n; i++ ){
		d[i] = s[i];
	}

	return dest;
}

WEAK void *memset( void *s, int c, size_t n ){
	uint8_t *dest = s;

	for ( size_t i = 0; i < n; i++ ){
		dest[i] = c;
	}

	return s;
}

WEAK size_t strlen( const char *s ){
	size_t i = 0;

	for ( ; s[i]; i++ );

	return i;
}

WEAK char *strcpy( char *dest, const char *src ){
	size_t i = 0;

	for ( ; src[i]; i++ ){
		dest[i] = src[i];
	}

	dest[i] = 0;

	return dest;
}

WEAK char *strncpy( char *dest, const char *src, size_t n ){
	size_t i = 0;

	for ( ; i < n && src[i]; i++ ){
		dest[i] = src[i];
	}

	for ( ; i < n; i++ ){
		dest[i] = '\0';
	}

	return dest;
}

WEAK size_t strlcpy( char *dest, const char *src, size_t n ){
	size_t i = 0;

	if ( n == 0 ){
		goto done;
	}

	for ( ; i < n - 1 && src[i]; i++ ){
		dest[i] = src[i];
	}

	dest[i] = '\0';

done:
	return strlen(src);
}

WEAK int strcmp( const char *s1, const char *s2 ){
	for ( ; *s1 && *s2; s1++, s2++ ){
		if ( *s1 == *s2 ) continue;
		if ( *s1 <  *s2 ) return -1;
		if ( *s1 >  *s2 ) return 1;
	}

	return 0;
}
