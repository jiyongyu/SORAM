
#include <stdio.h> // TODO DECPRECATED

#include <iostream>
using namespace std;

#include "util.hpp"

// -----------------------------------------------------------------------------
// Bit manipulation
// -----------------------------------------------------------------------------

// TODO optimize
int lg2(int v) 
{
   int r = 0;
   while (v >>= 1) r++;
   return r;
}

// TODO optimize
// return # of 1s in v
int pop_count(int v) 
{
   int c;
   for (c = 0; v; c++) v &= v - 1;
   return c;
}

// return v, rounded up to the next power of 2
uint32_t round_pow2(uint32_t v) 
{
   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v++;
   return v;
}

/* Random numbers ~ not tested yet.  Thanks to George Marsaglia.

#define znew (z=36969*(z&65535)+(z>>16))
#define wnew (w=18000*(w&65535)+(w>>16))
#define MWC ((znew<<16)+wnew )

typedef unsigned long UL;
// Global static variables:
static UL z=362436069, w=521288629;
// Use random seeds to reset z,w,jsr,jcong,a,b, and the table 

static UL x=0,y=0,bro; static unsigned char c=0;

// This is a test main program. It should compile and print 7
0's.
int main(void){
   int i; UL k;
   settable(12345,65435,34221,12345,9983651,95746118);
   for(i=1;i<1000001;i++){k=MWC ;} printf("%u\n", k- 904977562U);
}

*/

// -----------------------------------------------------------------------------
// Printers
// -----------------------------------------------------------------------------

void print_cache_line(cache_line_t * line) 
{
#if DEBUG
   int i;
   for (i = 0; i < LINE_SIZE / sizeof(word_t); i++)
      cout << (uint64_t) *(((word_t *) line->data) + i) << " ";
   //for (i = 0; i < LINE_SIZE; i++) printf(" %0x%1x ", *(cache->data + i));
#endif
}

void print_cache()
{
#if DEBUG
   int way;
   int i;
   cache_line_t * line;
   for (way = 0; way < __SW_CACHE_associativity; way++) 
   {
      printf ("-------------way%d----------------\n", way);
      line = (cache_line_t *) (memory + __memory_quantum * (2 * way + 1));
      for (i = 0; i < __SW_CACHE_set_size; i++) 
      {
         printf ("[set %d] line addr: %d, data:", i, line->addr);
         print_cache_line (line + i);
      }
   }
#endif
}

void print_ciphertext(cache_line_t * line) {
   for (int i = 0; i < LINE_SIZE; i++) 
      printf("%c", line->data[i]); 
}

// walk through memory up to allocation_size and printout when the memory 
// address space changes from reserved <-> sw
void print_address_spaces(int allocation_size)
{
#if DEBUG
   int i, sw_old, hw_old;
   sw_old = -1;
   hw_old = -1;
   for (i = 0; i < allocation_size; i++) 
   {
      data_t * a = memory + i;
      int sw_new = is_sw_memory(a);
      int hw_new = is_reserved_memory(a);
      // these are assuming a particular HW cache --- change as needed
      int block_offset = ((int64_t) a) & 0x3f;
      int block_set = (((int64_t) a) >> 6) & 0x1ff;
      if (sw_old != sw_new || hw_old != hw_new) 
      {
         sw_old = sw_new;
         hw_old = hw_new;
         // TODO convert to cout printf("[%d or %x] smem=%d rmem=%d set=%d offset=%d\n", i, a, sw_new, hw_new, block_set, block_offset);
      }
   }
#endif
}

// -----------------------------------------------------------------------------
// Common assertions
// -----------------------------------------------------------------------------

// is the candidate byte in the sw memory that we keep on chip?
bool_t is_sw_memory(data_t * candidate)
{
   ASSERT(candidate >= memory);
   int64_t delta = candidate - memory;
   int which_quantum = delta / __memory_quantum;

   // reserved quantum are always 0 aligned
   bool_t region_check = which_quantum & MMEM_UTILIZATION_MASK;

   // are we still on an address that can be permanently cached on chip?
   int num_res_quantum = (which_quantum >> MMEM_UTILIZATION_LG) + 1; 
   int sw_quantum_capacity = (llc_capacity - (llc_capacity >> MMEM_UTILIZATION_LG)) / __memory_quantum;
   bool_t capacity_check = (which_quantum - num_res_quantum) < sw_quantum_capacity;

   return region_check && capacity_check;
}

// is the candidate byte going to map to a reserved set?
bool_t is_reserved_memory(data_t * candidate) 
{
   ASSERT(candidate >= memory);
   int64_t delta = memory - candidate;
   int which_quantum = delta / __memory_quantum;
   return !(which_quantum & MMEM_UTILIZATION_MASK);
}

