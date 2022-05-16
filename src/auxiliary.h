#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma once

typedef uint64_t u64;
typedef uint8_t u8;
typedef char bool;

#define TRUE 1
#define FALSE 0

#define CHECK(v) do { if (v) return TRUE; } while (FALSE)

#define DEBUG(v) do { printf(__FILE__ ":%llu\t%llu\n", __LINE__, v); } while (FALSE)
#define ASSERT(v) do { if (!(v)) { DEBUG("Assertion Failed: " #v); exit(-1); }  } while (FALSE)

#define FATAL "\x1B[91m[FATAL]\x1B[0m\t"
#define ERR "\x1B[91m[ERROR]\x1B[0m\t"

#ifdef __INTELLISENSE__
#define WARN_UNUSED
#else
#define WARN_UNUSED __attribute__((warn_unused_result))
#endif

#define RESULT WARN_UNUSED bool

void *check_ptr(void *ptr);
void *allocate(u64 count, u64 size);
void *reallocate(void *ptr, u64 count, u64 size);
void deallocate(void *ptr);

char *read_file(const char *path);
