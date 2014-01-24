#ifndef __HOST_ACPI_H_INCLUDE__
#define __HOST_ACPI_H_INCLUDE__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <host/bitops.h>

#ifndef BUG_ON
#define BUG_ON(expr)	assert(!(expr))
#endif

#ifndef INT_MAX
#define INT_MAX		((int)(~0U>>1))
#endif
#ifndef INT_MIN
#define INT_MIN		(-INT_MAX - 1)
#endif

#ifndef LODWORD
#define LODWORD(w)			((uint32_t)w)
#endif
#ifndef HIDWORD
#define HIDWORD(w)			((uint32_t)((w)>>32))
#endif

#include <ctype.h>

typedef void *			caddr_t;

#define heap_alloc(size)	malloc(size)
#define heap_calloc(size)	calloc(1, size)
#define heap_realloc(ptr, size)	realloc(ptr, size)
#define heap_free(ptr)		free(ptr)

#endif /* __HOST_ACPI_H_INCLUDE__ */