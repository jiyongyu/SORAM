
#ifndef DEFS_H
#define DEFS_H

#include <inttypes.h>

// -----------------------------------------------------------------------------
// Parameters
// -----------------------------------------------------------------------------

// how much of the LLC do we reserve for data vs. reserved sets?
// 1/(1 << MMEM_UTILITY_LG) of the sets will be reserved
#ifndef MMEM_UTILIZATION_LG
#define MMEM_UTILIZATION_LG            1
#endif

#define LLC                            L2

// Size of memory in bytes.  NOTE: this memory size is independent of 
// MMEM_UTILIZATION_LG.  TODO figure out a way to use whole 48b address space
#define MEMORY_LG                      25

#ifndef WORKING_SET_LG
#define WORKING_SET_LG                 10
#endif

// The number of iterations to run per test (runtime should be >= .1 s for good 
// accuracy).
#ifndef WORK_LG
#define WORK_LG                        20
#endif

// Hardware cache (Nehalem 980 part -- Chris' desktop)
// DO NOT CHANGE unless you also change grind.py
#define LINE_SIZE                      64
#define L3_CAPACITY                    8388608
#define L3_ASSOC                       16
#define L2_CAPACITY                    262144
#define L2_ASSOC                       8
#define L1_CAPACITY                    32768
#define L1_ASSOC                       8

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef TIME
#define TIME 0
#endif

#ifndef CACHE_GRIND
#define CACHE_GRIND 0
#endif

#define WORKING_SET                    (1 << WORKING_SET_LG)
#define EXPERIMENT_ROUNDS              (max(1 << WORK_LG, WORKING_SET))

// used to determine if an address is in reserved space
#define MMEM_UTILIZATION_MASK          ((1 << MMEM_UTILIZATION_LG) - 1)

#define LINE_SIZE_LG                   (lg2(LINE_SIZE))
#define SET_SIZE_LG                    (__SW_CACHE_set_size_envelope_lg)

// Address fields for the SW cache (these can be tweaked as we like)
#define ADDRESS_WIDTH                  (sizeof(int32_t) * 8)
#define AFIELD_OFFSET_MASK             ((1 << LINE_SIZE_LG) - 1)
#define AFIELD_OFFSET_SHIFT            0
#define AFIELD_SET_MASK                ((1 << SET_SIZE_LG) - 1)
#define AFIELD_SET_SHIFT               LINE_SIZE_LG
#define AFIELD_TAG_SHIFT               LINE_SIZE_LG // this doesn't shift out the set bits because of how blocks are mapped to sets

// Cache fields
#define MASK_VALID                     0x1
#define SHIFT_VALID                    0
#define MASK_LRU                       0x7
#define SHIFT_LRU                      1

#define FALSE                          0
#define TRUE                           1

#define L1                             1
#define L2                             2
#define L3                             3

#define READ                           20
#define WRITE                          21

// Misc
#define ENC_MASK                       0x8000000000000000 // fake encryption that just sets the high bit to 1

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

typedef int32_t addr_t;
typedef int8_t data_t;
typedef int64_t word_t;
typedef int8_t bool_t;
typedef int64_t stat_t;

typedef struct {
   int32_t  addr;
   int8_t   status; // valid bit, LRU information, what else?
   data_t   data[LINE_SIZE]; // TODO make actual line smaller so that we fit into a cache line?
} cache_line_t;

#endif
