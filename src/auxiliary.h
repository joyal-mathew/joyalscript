#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int64_t i64;
typedef uint64_t u64;
typedef uint16_t u16;
typedef uint8_t u8;
typedef char bool;

#define TRUE 1
#define FALSE 0

#define CHECK(v) do { if (v) return TRUE; } while (FALSE)

#define DEBUG(v) do { printf(__FILE__ ":%llu\t%llu\n", __LINE__, v); } while (FALSE)
#define ASSERT(v) do { if (!(v)) { printf(__FILE__ ":%llu\tAssertion Failed: " #v "\n", __LINE__); exit(-1); }  } while (FALSE)

#define FATAL "\x1B[91m[FATAL]\x1B[0m\t"
#define ERR "\x1B[91m[ERROR]\x1B[0m\t"

#ifdef __INTELLISENSE__
#define WARN_UNUSED
#define PACKED
#define UNUSED
#define _Static_assert(...)
#else
#define WARN_UNUSED __attribute__((warn_unused_result))
#define UNUSED __attribute__((unused))
#define PACKED __attribute__((__packed__))
#endif

#define RESULT WARN_UNUSED bool

void begin_tracking();
void tracking_diagnostics();
void *check_ptr(void *ptr);
void *heap_alloc(u64 count, u64 size);
void *heap_realloc(void *ptr, u64 count, u64 size);
void heap_dealloc(void *ptr);

char *read_file(const char *path);
