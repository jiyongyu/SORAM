
#ifndef GLOBALS_H
#define GLOBALS_H

/*
   Public constants (i.e., can be leaked to the adversary)
*/

#include "defs.hpp"

extern data_t * memory_raw;
extern data_t * memory;
extern addr_t memory_size;

extern int llc_associativity;
extern int llc_capacity_way;
extern int llc_capacity;

// the amount of contiguous memory in a reserved region
extern addr_t __memory_quantum;
extern addr_t __memory_quantum_mask;

// the amount of reserved memory in a sw scratchpad region
extern addr_t __sw_memory_quantum;

// a simple 1-element cache
extern cache_line_t * cache;
extern addr_t __SW_CACHE_set_size;
extern addr_t __SW_CACHE_set_size_envelope_lg;
extern addr_t __SW_CACHE_associativity;

#if DEBUG
extern stat_t cache_bytes_touched;
extern stat_t cache_accesses;
extern stat_t cache_hits;
extern stat_t cache_misses;
extern stat_t cache_sets_searched;
extern stat_t cache_evictions;
#endif

#endif
