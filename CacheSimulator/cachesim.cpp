/*
 * CS3375 Computer Architecture
 * Course Project
 * Cache Simulator Design and Development
 * FALL 2017
 * Modified by Darien Sokolov
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>  // Imported to use bool
#include <string.h>
#include <math.h>
#include "cachesim.h"

int main(int argc, char *argv[]) {
  /*if (argc != 3) {
      printf("Usage: %s generic <trace file name>\n", argv[0]);
      return 1;
  }*/
  
  //char* trace_file_name = argv[2];

  //if (strncmp(argv[1], "generic", 6)==0) { /* Simulating generic cache */

  //}

  runTest(64, 32768, 1, "trace.hpcg");
    
  return 0;
}

void runTest(const int block_size, const int cache_size, const int way_size, const char* trace_file_name) {
  const int num_blocks = (cache_size / block_size);
  const int num_sets = (num_blocks / way_size);

  printf("BLOCK SIZE = %d Bytes\n", block_size);
  printf("%d-WAY\n", way_size);
  printf("CACHE SIZE = %d Bytes\n", cache_size);
  printf("NUMBER OF BLOCKS = %d\n", num_blocks);
  printf("NUMBER OF SETS = %d\n", num_sets);
  printf("\n");

  unsigned *valid_field = new unsigned[num_blocks];
  unsigned *dirty_field = new unsigned[num_blocks];
  uint64_t *tag_field = new uint64_t[num_blocks];
  n_way_cache cache(valid_field, dirty_field, tag_field, block_size, cache_size, num_blocks, way_size, num_sets);
  char mem_request[20];
  uint64_t address;
  FILE *fp;

  for (int i = 0; i<num_blocks; i++) { /* Initialization */
    cache.valid_field[i] = 0;
    cache.dirty_field[i] = 0;
    cache.tag_field[i] = 0;
  }
  cache.hits = 0;
  cache.misses = 0;
  cache.accessCount = 0;

  fp = fopen(trace_file_name, "r"); /* Opening the memory trace file */

  while (fgets(mem_request, 20, fp) != NULL) { /* Read the memory request address and access the cache */
    address = convert_address(mem_request);
    n_way_cache_access(&cache, address);
  }
  printf("\n==================================\n"); /* Print out the results */
  printf("Cache type:    Generic Cache\n");
  printf("==================================\n");
  printf("Cache Hits:    %d\n", cache.hits);
  printf("Cache Misses:  %d\n", cache.misses);
  printf("Cache Hit Rate: %f\n", cache.hitRate);
  printf("Cache Miss Rate: %f\n", cache.missRate);
  printf("Cache Access Count %d\n", cache.accessCount);
  printf("\n");

  fclose(fp);
}

uint64_t convert_address(char memory_addr[]) { /* Converts the physical 32-bit address in the trace file to the "binary" */
    uint64_t binary = 0;                       /* (a uint64 that can have bitwise operations on it) */
    int i = 0;

    while (memory_addr[i] != '\n') {
        if (memory_addr[i] <= '9' && memory_addr[i] >= '0') {
            binary = (binary*16) + (memory_addr[i] - '0');
        } else {
            if(memory_addr[i] == 'a' || memory_addr[i] == 'A') {
                binary = (binary*16) + 10;
            }
            if(memory_addr[i] == 'b' || memory_addr[i] == 'B') {
                binary = (binary*16) + 11;
            }
            if(memory_addr[i] == 'c' || memory_addr[i] == 'C') {
                binary = (binary*16) + 12;
            }
            if(memory_addr[i] == 'd' || memory_addr[i] == 'D') {
                binary = (binary*16) + 13;
            }
            if(memory_addr[i] == 'e' || memory_addr[i] == 'E') {
                binary = (binary*16) + 14;
            }
            if(memory_addr[i] == 'f' || memory_addr[i] == 'F') {
                binary = (binary*16) + 15;
            }
        }
        i++;
    }
    DebugPrint("%s converted to %llu\n", memory_addr, binary);
    return binary;
}

void n_way_cache_access(n_way_cache *cache, uint64_t address) { // An N-way cache can be used to accomplish a Direct-mapped cache or a Fully-associative cache
    uint64_t block_addr = address >> (unsigned)log2(cache->BLOCK_SIZE);
    uint64_t index = block_addr % cache->NUM_BLOCKS;
    uint64_t tag = block_addr >> (unsigned)log2(cache->NUM_BLOCKS);
    uint64_t firstBlockInSet = index % cache->NUM_SETS;  // First block in the set of interest
    uint64_t blockInSetIndex = firstBlockInSet; // Temporary block index, used to indicate block of interest within a set
    bool miss, replaced;  // Bool variables used as helpers in writing conditional logic
    DebugPrint("Memory address: %llu, Block address: %llu, Index: %llu, Tag: %llu ", address, block_addr, index, tag);
    
    cache->accessCount++; // Count the number of times the cache has been accessed
    for (int blockNumber = 0; blockNumber < cache->WAY_SIZE; blockNumber++) {
      blockInSetIndex = firstBlockInSet + (blockNumber * cache->NUM_SETS);
      if (cache->valid_field[blockInSetIndex] && cache->tag_field[blockInSetIndex] == tag) { /* Cache hit */
        cache->hits++;
        DebugPrint("Hit!\n");
        miss = false;
        break;
      } else { /* Cache miss */
        DebugPrint("Miss!\n");
        if (cache->valid_field[blockInSetIndex] && cache->dirty_field[blockInSetIndex]) { /* Write the cache block back to memory */
        }
        miss = true;
      }
    }
    replaced = false;
    if (miss) { // If the cache access resulted in a miss
      cache->misses++;
      for (int blockNumber = 0; blockNumber < cache->WAY_SIZE; blockNumber++) {  // Find a cache line within the set of interest that has not been recently used
        blockInSetIndex = firstBlockInSet + (blockNumber * cache->NUM_SETS);
        if (!cache->valid_field[blockInSetIndex]) { // If NRU line found, replace it
          cache->tag_field[blockInSetIndex] = tag;
          cache->valid_field[blockInSetIndex] = 1;
          cache->dirty_field[blockInSetIndex] = 0;
          replaced = true;
          break;
        }
      }
      if (!replaced) {  // If no replacements were made (due to no NRU lines found)
        for (int blockNumber = 0; blockNumber < cache->WAY_SIZE; blockNumber++) {  // Set all lines within the set to be NRU
          blockInSetIndex = firstBlockInSet + (blockNumber * cache->NUM_SETS);
          cache->valid_field[blockInSetIndex] = 0;
        }
        cache->tag_field[firstBlockInSet] = tag;  // Replace an arbitrary/random line
        cache->valid_field[firstBlockInSet] = 1;
        cache->dirty_field[firstBlockInSet] = 0;
      }
    }
    cache->hitRate = (double)cache->hits / cache->accessCount;  // Calculate new hitRate
    cache->missRate = (double)cache->misses / cache->accessCount; // Calculate new missRate
}

