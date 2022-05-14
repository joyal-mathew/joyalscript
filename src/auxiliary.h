#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma once

typedef uint64_t u64;
typedef char bool;

#define TRUE 1
#define FALSE 0

#define CHECK(v) do { if (v) return TRUE; } while (FALSE)

#define DEBUG(s) do { printf(__FILE__ ":%llu\t" s "\n", __LINE__); } while (FALSE)

#define FATAL "[FATAL]\t"
#define ERROR "[ERROR]\t"

void *check_ptr(void *ptr);
void *allocate(u64 count, u64 size);
void *reallocate(void *ptr, u64 count, u64 size);
void deallocate(void *ptr);
