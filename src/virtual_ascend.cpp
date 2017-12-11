
#include <time.h>
#include <stdio.h> // TODO DECPRECATED
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <inttypes.h>

#include "sys/ktiming.h"
#include "dram.hpp"
#include "cache.hpp"

// -----------------------------------------------------------------------------
// Program initialization
// -----------------------------------------------------------------------------

void initialize_put() 
{
   ASSERT( MEMORY_LG >= WORKING_SET_LG );

#if DEBUG
   cache_bytes_touched = 0;
   cache_accesses = 0;
   cache_hits = 0;
   cache_misses = 0;
   cache_sets_searched = 0;
   cache_evictions = 0;
#endif

   // allocate the reserved sets and memory space offsets
   if (LLC == L1)
   {
      llc_capacity = L1_CAPACITY;
      llc_associativity = L1_ASSOC;
   } else if (LLC == L2) 
   {
      llc_capacity = L2_CAPACITY;
      llc_associativity = L2_ASSOC;
   } else if (LLC == L3) 
   {
      llc_capacity = L3_CAPACITY;
      llc_associativity = L3_ASSOC;
   }
   llc_capacity_way = llc_capacity / llc_associativity;

   // Initialize memory quantums: the size of reserved/sw chunks of contiguous 
   // memory
   __memory_quantum = llc_capacity_way / (1 << MMEM_UTILIZATION_LG);
   __sw_memory_quantum = llc_capacity_way - __memory_quantum;
   ASSERT( !(__sw_memory_quantum % __memory_quantum) );
   ASSERT( pop_count(__memory_quantum) == 1 );
   __memory_quantum_mask = __memory_quantum - 1;

   //
   // Allocate all of main memory using normal memory allocation ---------------
   //

   // check 32b address space overflow
   ASSERT( (sizeof(addr_t) * 8 - 1) > (MMEM_UTILIZATION_LG + MEMORY_LG));
   addr_t requested_memory = (1 << MEMORY_LG);
   addr_t allocation_size = requested_memory;
   printf("??%d\n", allocation_size);
   allocation_size *= 1 + (llc_capacity / allocation_size);
   printf("??%d\n", allocation_size);
   // we still wanted requested_memory after initialization shift
   while (allocation_size - llc_capacity < requested_memory) 
      allocation_size <<= 1;
   allocation_size <<= MMEM_UTILIZATION_LG; // extra space for sparse DRAM
   allocation_size *= sizeof(data_t);
   memory_raw = (data_t *) malloc(allocation_size);

   // Initialization shift
   // Get a pointer to memory such that memory will point to a cache line that
   // maps to set 0 in the LLC
   memory = memory_raw + llc_capacity;
   memory = memory - ( ((int64_t) memory) % llc_capacity);
   // check llc alignment
   ASSERT( !( ((int64_t) memory) & (llc_capacity - 1)) );
   memory_size = (memory_raw + allocation_size) - memory;

   // --------------------------------------------------------------------------

   // Initialize crypto tools
   // TODO store key somewhere
   crypto_initialize();

   // TODO pad memory so that stack is in user sets?

   // TODO pad instructions?

   // Initialize the cache dimensions
   __SW_CACHE_set_size = __sw_memory_quantum / sizeof(cache_line_t);
   __SW_CACHE_set_size_envelope_lg = lg2(round_pow2(__SW_CACHE_set_size));
   __SW_CACHE_associativity = llc_associativity - 1; // TODO make this more general?

   // Touch every line in the cache to bring it into the LLC
   cache_initialize();

#if DEBUG
   cout << "Protecting: L" << LLC << " cache" << endl;
   cout <<  "Memory quantum: reserved=" << 
            __memory_quantum << ", sw=" << 
            __sw_memory_quantum << endl;
   cout <<  "Memory raw/offset: "<< memory_raw << "/" << 
            memory - memory_raw << endl;
   cout <<  "Memory lb/ub: " << memory << "/" << 
            memory_raw + memory_size << " (size = " << 
            memory_size << ")" << endl;
   cout <<  "SW Cache line size (struct) = " << sizeof(cache_line_t) << 
            ", data capacity in SW cache = " << 
            __SW_CACHE_associativity * LINE_SIZE * __SW_CACHE_set_size << 
            " B, total capacity in SW cache = " << cache_bytes_touched << 
            " B" << endl;
   cout <<  "Program information: working set = " << WORKING_SET << 
            ", rd/wr rounds = " << EXPERIMENT_ROUNDS << endl;

   //print_address_spaces(allocation_size);
#endif

#if DEBUG > 1
      printf("Loading program ...\n");
#endif

   // load the user program / its data into dram
   int i;
   cache_line_t tmp_line;
   for (i = 0; i < WORKING_SET; i += LINE_SIZE) 
   {
      tmp_line.addr = i;
      memset(tmp_line.data, 0, LINE_SIZE);
      *((word_t*) &tmp_line.data) = i / LINE_SIZE;
      dram_write(&tmp_line);
   }

/* #if DEBUG */
/*         printf ("----------------------------------------------------\n"); */
/*    for (i = 0; i < WORKING_SET; i += LINE_SIZE) { */
/*       cache->addr = i; */
/*       dram_read(cache); */
/*    } */
/*         printf ("----------------------------------------------------\n"); */
/* #endif */

}

// -----------------------------------------------------------------------------
// Toy program
// -----------------------------------------------------------------------------

// cache line stride
#define ADDR(i) ( (i << LINE_SIZE_LG) & (WORKING_SET-1) )

// unit stride
//#define ADDR(i) ( i & (WORKING_SET-1)  )

// PR access pattern
//#define ADDR(i) ( ((i + 523)*253573) & (WORKING_SET-1) & ~(LINE_SIZE-1) )

float run_put()
{
   int i, j;

#if DEBUG || TIME
	clockmark_t time1 = ktiming_getmark();
#endif

    word_t tmp = 0;
    int addr = 0;
    for (i = 0; i < EXPERIMENT_ROUNDS; i++) {
        addr = ADDR(i);
        tmp = memory_read(addr);
        tmp += 1;
        memory_write(tmp, addr, -1);
    }
#if DEBUG || TIME
   clockmark_t time2 = ktiming_getmark();
   return ktiming_diff_sec(&time1, &time2);
#endif
   return 0.0;
}

float check_performance()
{
   int i;
   data_t * mem = (data_t *) malloc(1 << MEMORY_LG);

	clockmark_t time1 = ktiming_getmark();

   for (i = 0; i < EXPERIMENT_ROUNDS; i++) 
   {      
      word_t tmp;
      int addr = ADDR(i);
      tmp = mem[addr];
      tmp += 1;
      mem[addr] = tmp;
   }

	clockmark_t time2 = ktiming_getmark();
   free(mem);

   return ktiming_diff_sec(&time1, &time2);
}

void check_put() 
{
   int i;

   printf("Checking memory ...\n");

   word_t sum_actual = 0;
   cache_line_t tmp_line;
   for (i = 0; i < WORKING_SET; i += LINE_SIZE) 
   {
      tmp_line.addr = i;
      dram_read(&tmp_line);
      word_t d = *((word_t *) &(tmp_line.data));
      sum_actual += d;
   }

   word_t sum_expected = EXPERIMENT_ROUNDS;

   if (sum_actual == sum_expected) 
   {
      printf("\033[1;32mPASS\033[m\n");
   } else 
   {
      printf("\033[1;31mFAIL\033[m\n");
      cout << "Sum actual,expected = " << sum_actual << "," << sum_expected << endl;
      assert(0);
   }
}

// -----------------------------------------------------------------------------
// Interpretter
// -----------------------------------------------------------------------------

int main()
{
   initialize_put();

   float sw_ascend_runtime = run_put();
   cache_flush();

#if DEBUG
   //check_put();
   ASSERT(cache_accesses == cache_hits + cache_misses);
   float cache_miss_rate = ((float) cache_misses) / ((float) cache_accesses);
   float avg_cache_sets_searched = ((float) cache_sets_searched) / ((float) cache_accesses);
   cout  << "Cache accesses = " << cache_accesses << ", avg sets searched per access=" 
         << avg_cache_sets_searched << ", miss rate =" << cache_miss_rate << endl;
#endif

   free(memory_raw);

#if DEBUG || TIME
   float native_runtime = check_performance();
   float slowdown = sw_ascend_runtime / native_runtime;
	printf(  "%d,%d,%d,%d,%f,%f,%f\n", 
            LLC, MMEM_UTILIZATION_LG, WORKING_SET_LG, WORK_LG, 
            sw_ascend_runtime, native_runtime, slowdown);
#endif

   return 0;
}

