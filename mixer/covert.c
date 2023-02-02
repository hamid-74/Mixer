#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define ASSOCIATIVITY 8

#define L1_CACHE_SIZE (32*1024)
#define LINE_SIZE 64
#define ASSOCIATIVITY 8
#define L1_NUM_SETS (L1_CACHE_SIZE/(LINE_SIZE*ASSOCIATIVITY))
#define NUM_OFFSET_BITS 6
#define NUM_INDEX_BITS 6
#define NUM_OFF_IND_BITS (NUM_OFFSET_BITS + NUM_INDEX_BITS)


//__attribute__ ((aligned (64))) uint64_t spy_array[4096];
//__attribute__ 
uint64_t trojan_array[32*4096];

inline __attribute__((always_inline)) uint64_t* get_eviction_set_address(uint64_t *base, int set, int way)
{
    uint64_t tag_bits = (((uint64_t)base) >> NUM_OFF_IND_BITS);
    int idx_bits = (((uint64_t)base) >> NUM_OFFSET_BITS) & 0x3f;

    if (idx_bits > set) {
        return (uint64_t *)((((tag_bits << NUM_INDEX_BITS) +
                               (L1_NUM_SETS + set)) << NUM_OFFSET_BITS) +
                            (L1_NUM_SETS * LINE_SIZE * way));
    } else {
        return (uint64_t *)((((tag_bits << NUM_INDEX_BITS) + set) << NUM_OFFSET_BITS) +
                            (L1_NUM_SETS * LINE_SIZE * way));
    }
}



/* This function sets up a trojan/spy eviction set using the
 * function above.  The eviction set is essentially a linked
 * list that spans all ways of the conflicting cache set.
 *
 * i.e., way-0 -> way-1 -> ..... way-7 -> NULL
 *
 */
inline __attribute__((always_inline)) void setup(uint64_t *base, int assoc)
{
    uint64_t i, j;
    uint64_t *eviction_set_addr;

    // Prime the cache set by set (i.e., prime all lines in a set)
    for (i = 0; i < L1_NUM_SETS; i++) {
        eviction_set_addr = get_eviction_set_address(base, i, 0);
        for (j = 1; j < assoc; j++) {
            *eviction_set_addr = (uint64_t)get_eviction_set_address(base, i, j);
            eviction_set_addr = (uint64_t *)*eviction_set_addr;  // sets up a chain of dereferences to get addresses
        }
        *eviction_set_addr = 0;
    }
}

int main()
{


    
    setup(trojan_array, ASSOCIATIVITY*8);


    return 0;

    //setup(spy_array, ASSOCIATIVITY);

}





