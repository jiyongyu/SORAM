
#ifndef CACHE_H
#define CACHE_H

#include "util.hpp"
#include "defs.hpp"

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

bool_t is_valid(cache_line_t * line);

void set_valid(cache_line_t * line);
void clear_valid(cache_line_t * line);

addr_t get_offset(addr_t a1);
addr_t get_set(addr_t a1);
addr_t get_tag(addr_t a1);

int get_lru (cache_line_t * line);
void set_lru (cache_line_t * line, int lru);

bool_t tag_check(addr_t a1, addr_t a2);

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

word_t memory_read(addr_t addr);
void memory_write(word_t data, addr_t addr, word_t mask);

void cache_initialize();
void cache_flush();

#endif
