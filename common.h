#ifndef _COMMON_H
#define _COMMON_H

#include "ccan/endian/endian.h"

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define min(x, y) ((x) > (y) ? (y) : (x))
#define max(x, y) ((x) > (y) ? (x) : (y))

#define __stringify_1(x...) #x
#define __stringify(x...)  __stringify_1(x)

static inline uint32_t mmio_read32(void *addr)
{
	leint32_t *p = addr;

	return le32_to_cpu(*p);
}

/* Access 64-bit registers as 2 32-bit; Some devices fail 64-bit MMIO. */
static inline uint64_t mmio_read64(void *addr)
{
	const volatile uint32_t *p = addr;
	uint32_t low, high;

	low = le32_to_cpu(*p);
	high = le32_to_cpu(*(p + 1));

	return ((uint64_t) high << 32) | low;
}

#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

#endif
