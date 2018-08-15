#ifndef _C4RT_PRNG_H
#define _C4RT_PRNG_H 1

#include <stdint.h>
#include <stdbool.h>

#define C4RT_PRNG_DEFAULT_SEED 0xabadc0debabe

void c4rt_prng_seed(uint64_t new_seed);
uint64_t c4rt_prng_get_seed(void);

bool     c4rt_prng_bool(void);
uint8_t  c4rt_prng_u8(void);
uint16_t c4rt_prng_u16(void);
uint32_t c4rt_prng_u32(void);
uint64_t c4rt_prng_u64(void);

#endif
