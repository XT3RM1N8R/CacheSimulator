/*
 * CS3375 Computer Architecture
 * Course Project
 * Cache Simulator Design and Development
 * FALL 2017
 * Modified by Darien Sokolov
 */

//#define DBG                   /* Define this in order to enable compilation of debug printing to console */
#ifdef DBG                   
  #define DebugPrint(x) print(x)  /* If DBG is defined, turn all DebugPrint calls into standard printf calls */
#else
  #define DebugPrint(x) /* Nothing */ /* This essentially removes all calls to DebugPrint(x) before compiling, if DBG is not defined */
#endif

/*The data structure of generic cache*/
class n_way_cache {
public:/*
  n_way_cache() {
    n_way_cache(nullptr, nullptr, nullptr);
  }*/

  n_way_cache(unsigned *valid_field, unsigned *dirty_field, uint64_t *tag_field, int BLOCK_SIZE, int CACHE_SIZE, int NUM_BLOCKS, int WAY_SIZE, int NUM_SETS) {
    this->valid_field = valid_field;
    this->dirty_field = dirty_field;
    this->tag_field = tag_field;
    this->BLOCK_SIZE = BLOCK_SIZE;
    this->CACHE_SIZE = CACHE_SIZE;
    this->NUM_BLOCKS = NUM_BLOCKS;
    this->WAY_SIZE = WAY_SIZE;
    this->NUM_SETS = NUM_SETS;
  }

  int BLOCK_SIZE;           /* Cache block size (or cache line size) in bytes
                               (must be power of 2). 4 Bytes = 1 Word */
  int CACHE_SIZE;           /* Cache capacity in bytes (must be power of 2) */
  int NUM_BLOCKS;
  int WAY_SIZE;             /* Associativity */
  int NUM_SETS;

  unsigned *valid_field;   /* Valid field */
  unsigned *dirty_field;   /* Dirty field; since we don't distinguish writes and \\
                              reads in this project yet, this field doesn't really matter */
  uint64_t *tag_field;     /* Tag field */
  //char data_field[NUM_BLOCKS][BLOCK_SIZE];  /* Data field; since we don't really fetch data, \\
  //                                             this field doesn't really matter */
  int hits;                          /* Hit count */
  int misses;                        /* Miss count */
  double hitRate;                    /* Hit rate */
  double missRate;                   /* Miss rate */
  int accessCount;                   /* Total number of cache accesses */
};


/*Read the memory traces and convert it to binary*/
uint64_t convert_address(char memory[]);

/*Simulate the generic cache*/
void n_way_cache_access(n_way_cache *cache, uint64_t address);

/*Run a cache test with the given arguments*/
void runTest(const int block_size, const int cache_size, const int way_size, const char* trace_file_name);

