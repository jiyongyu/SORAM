
#ifndef UTIL_H
#define UTIL_H

#include <assert.h>

#include <iostream>
using namespace std;

#include "globals.h"
#include "defs.hpp"

#define __ASCEND_max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// Even if -g is off, assert() calls are SLOW AS HELL; so get rid of them
#define ASSERT(args) if (!TIME) assert(args);

int lg2(int v);
int pop_count(int v);
uint32_t round_pow2(uint32_t v);

void print_cache_line(cache_line_t * line);
void print_cache();
void print_ciphertext(cache_line_t * line);
void print_address_spaces();

bool_t is_sw_memory(data_t * set);
bool_t is_reserved_memory(data_t * set);

#endif
