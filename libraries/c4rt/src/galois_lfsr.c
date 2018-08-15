// 64 bit galois linear feedback shift register
#include <c4rt/prng.h>
#include <stdbool.h>
#include <stdint.h>

static uint64_t state = C4RT_PRNG_DEFAULT_SEED;

static inline uint64_t tap(uint64_t state, unsigned bit){
	return state >> (64 - bit);
}

static inline uint64_t tapbit(unsigned bit){
	return (uint64_t)1 << (bit - 1);
}

static inline uint64_t lfsr_next(uint64_t state){
	uint64_t bit = state & 1;
	
	state >>= 1;

	if (bit) {
		state ^= (tapbit(64) | tapbit(63) | tapbit(61) | tapbit(60));
	}

	return state;
}

static inline bool lfsr_output( uint64_t state ){
	return state & 1;
}

void c4rt_prng_seed(uint64_t new_seed){
	state = new_seed;
}

uint64_t c4rt_prng_get_seed(void){
	return state;
}

bool c4rt_prng_bool(void){
	bool ret = lfsr_output(state);
	state = lfsr_next(state);

	return ret;
}

uint8_t c4rt_prng_u8(void){
	uint8_t ret = 0;

	for (unsigned i = 0; i < 8; i++) {
		ret <<= 1;
		ret |= c4rt_prng_bool();
	}

	return ret;
}

uint16_t c4rt_prng_u16(void){
	return ((uint16_t)c4rt_prng_u8() << 8) | c4rt_prng_u8();
}

uint32_t c4rt_prng_u32(void){
	return ((uint32_t)c4rt_prng_u16() << 16) | c4rt_prng_u16();
}

uint64_t c4rt_prng_u64(void){
	return ((uint64_t)c4rt_prng_u32() << 32) | c4rt_prng_u32();
}
