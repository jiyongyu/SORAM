
#include "globals.h"

data_t * memory_raw = 0;
data_t * memory = 0;
addr_t memory_size = 0;

int llc_associativity = 0;
int llc_capacity_way = 0;
int llc_capacity = 0;

// the amount of contiguous memory in a reserved region
addr_t __memory_quantum = 0;
addr_t __memory_quantum_mask = 0;

// the amount of reserved memory in a sw scratchpad region
addr_t __sw_memory_quantum = 0;

// a simple 1-element cache
cache_line_t * cache = 0;
addr_t __SW_CACHE_set_size = 0;
addr_t __SW_CACHE_set_size_envelope_lg = 0;
addr_t __SW_CACHE_associativity = 0;

#if DEBUG
stat_t cache_bytes_touched = 0;
stat_t cache_accesses = 0;
stat_t cache_hits = 0;
stat_t cache_misses = 0;
stat_t cache_sets_searched = 0;
stat_t cache_evictions = 0;
#endif

