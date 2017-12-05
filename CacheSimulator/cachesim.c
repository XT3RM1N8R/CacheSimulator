/*
 * CS3375 Computer Architecture
 * Course Project
 * Cache Simulator Design and Development
 * FALL 2017
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "cachesim.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <direct> <trace file name>\n", argv[0]);
        return 1;
    }

    printf("BLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
    printf("%d-WAY\n", WAY_SIZE);
    printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
    printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
    printf("NUMBER OF SETS = %d\n", NUM_SETS);
    printf("\n");

    char* trace_file_name = argv[2];
    struct direct_mapped_cache d_cache;
    char mem_request[20];
    uint64_t address;
    FILE *fp;

    for (int i=0; i<NUM_BLOCKS; i++) { /* Initialization */
        d_cache.valid_field[i] = 0;
        d_cache.dirty_field[i] = 0;
        d_cache.tag_field[i] = 0;
    }
    d_cache.hits = 0;
    d_cache.misses = 0;
    d_cache.accessCount = 0;

    fp = fopen(trace_file_name, "r"); /* Opening the memory trace file */

    if (strncmp(argv[1], "direct", 6)==0) { /* Simulating direct-mapped cache */
        while (fgets(mem_request, 20, fp)!= NULL) { /* Read the memory request address and access the cache */
            address = convert_address(mem_request);
            direct_mapped_cache_access(&d_cache, address);
        }
        printf("\n==================================\n"); /* Print out the results */
        printf("Cache type:    Direct-Mapped Cache\n");
        printf("==================================\n");
        printf("Cache Hits:    %d\n", d_cache.hits);
        printf("Cache Misses:  %d\n", d_cache.misses);
        printf("Cache Hit Rate: %f\n", d_cache.hitRate);
        printf("Cache Miss Rate: %f\n", d_cache.missRate);
        printf("Cache Access Count %d\n", d_cache.accessCount);
        printf("\n");
    }

    fclose(fp);
    return 0;
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

void direct_mapped_cache_access(struct direct_mapped_cache *cache, uint64_t address) {
    uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
    uint64_t index = block_addr % NUM_BLOCKS;
    uint64_t tag = block_addr >> (unsigned)log2(NUM_BLOCKS);
    uint64_t firstBlockInSet = index % NUM_SETS;
    uint64_t blockInSetIndex = firstBlockInSet;
    bool miss, replaced;
    DebugPrint("Memory address: %llu, Block address: %llu, Index: %llu, Tag: %llu ", address, block_addr, index, tag);
    
    cache->accessCount++;
    for (int blockNumber = 0; blockNumber < WAY_SIZE; blockNumber++) {
      blockInSetIndex = blockInSetIndex + (blockNumber * NUM_SETS);
      if (cache->valid_field[blockInSetIndex] && cache->tag_field[blockInSetIndex] == tag) { /* Cache hit */
        cache->hits++;
        DebugPrint("Hit!\n");
        miss = false;
        break;
      } else { /* Cache miss */
        DebugPrint("Miss!\n");
        if (cache->valid_field[blockInSetIndex] && cache->dirty_field[blockInSetIndex]) { /* Write the cache block back to memory */
        }
        cache->tag_field[blockInSetIndex] = tag;
        cache->valid_field[blockInSetIndex] = 1;
        cache->dirty_field[blockInSetIndex] = 0;
        miss = true;
      }
    }
    replaced = false;
    if (miss) {
      cache->misses++;
      blockInSetIndex = firstBlockInSet;
      for (int blockNumber = 0; blockNumber < WAY_SIZE; blockNumber++) {
        blockInSetIndex = blockInSetIndex + (blockNumber * NUM_SETS);
        if (!cache->valid_field[blockInSetIndex]) {
          cache->tag_field[blockInSetIndex] = tag;
          cache->valid_field[blockInSetIndex] = 1;
          cache->dirty_field[blockInSetIndex] = 0;
          replaced = true;
          break;
        }
      }
      if (!replaced) {
        blockInSetIndex = firstBlockInSet;
        for (int blockNumber = 0; blockNumber < WAY_SIZE; blockNumber++) {
          blockInSetIndex = blockInSetIndex + (blockNumber * NUM_SETS);
          if (!cache->valid_field[blockInSetIndex]) {
            cache->valid_field[blockInSetIndex] = 0;
          }
        }
        cache->tag_field[firstBlockInSet] = tag;
        cache->valid_field[firstBlockInSet] = 1;
        cache->dirty_field[firstBlockInSet] = 0;
      }
    }
    cache->hitRate = (double)cache->hits / cache->accessCount;
    cache->missRate = (double)cache->misses / cache->accessCount;
}

