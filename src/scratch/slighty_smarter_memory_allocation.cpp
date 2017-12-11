// -----------------------------------------------------------------------------
// Old code: smart memory allocation -- idea use realloc to get a bit more memory when memory_raw isn't aligned
// -----------------------------------------------------------------------------

/*
   // How much memory to allocate
   ASSERT( (sizeof(addr_t) * 8 - 1) > (MMEM_UTILIZATION_LG + MEMORY_LG));
   addr_t allocation_size = (1 << MEMORY_LG);
   allocation_size <<= MMEM_UTILIZATION_LG;
   allocation_size *= sizeof(data_t);

   addr_t addendum = 0;
   memory_raw = NULL;
   while (TRUE) {
      // try to allocate WORKING_SET amount of memory
      memory_raw = (data_t *) realloc(memory_raw, allocation_size + addendum);
      ASSERT( memory_raw != NULL );

      // do we have an alignment issue?
      addr_t unalignment = ((int64_t) memory_raw + addendum) % llc_capacity;
      printf("unalign: %d, new %p\n", unalignment, memory_raw + (llc_capacity - unalignment));
      if (unalignment) {
         // if so, how much will we have to bump memory up?
         addendum = llc_capacity - unalignment;
         ASSERT( !( ((int64_t) (memory_raw + addendum)) & (llc_capacity - 1)) );
      } else {
         memory = memory_raw + addendum;
         ASSERT( !( ((int64_t) memory) & (llc_capacity - 1)) );
         break;
      }
   }

   // Get a pointer to memory such that memory will point to a cache line that
   // maps to set 0 in the LLC


   //effective_memory_size = allocation_size - (memory - memory_raw);
   //effective_working_set = allocation_size >> (;
*/

