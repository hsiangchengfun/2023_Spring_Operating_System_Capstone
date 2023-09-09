#ifndef	_MM_H_
#define	_MM_H_

#include "utils.h"

//usable memory region is from 0x00 to 0x3C000000
#define MEM_START 0x00
#define MEM_END 0x3C000000
// #define MEM_START 0x10000000
// #define MEM_END   0x20000000
//an unused memory region (e.g. 0x1000_0000 -> 0x2000_0000)
#define FRAME_SIZE 4096


#define MAX_LEVEL 6


#define ALLOCABLE           0
#define ALLOCATED           -1
#define LARGER_CONTI        -2
#define RESERVED            -3

#define NULL                0

#define MAX_POOL_PAGES      8
#define MAX_POOLS           8
#define MIN_CHUNK_SIZE      8

#define MAX_RESERVABLE      8


struct frame{

    uint32_t index;
    struct frame *prev , *next;
    int order;
    int stat;

};

struct node{
    struct  node *prev, *next;
};

struct memory_pool{

    uint32_t frame_used;
    uint32_t chunk_used;
    uint32_t chunk_size;
    uint32_t chunk_per_frame;
    uint32_t chunk_offset_each_frame;
    struct node* unallocated_ptr;
    void *frame_base_addrs[MAX_POOL_PAGES];



};

struct reserved_node
{
	struct reserved_node* next;
    struct int_pair pair;
};



void init_mm();
void *malloc(uint32_t size);
void free_memory(void* addr);
void check_state();
void *dynamic_alloc(uint32_t size);
void dynamic_free(void* addr);
int get_chunk(uint32_t size);
void init_pool(struct memory_pool*, uint32_t size);
void reserved_memory(void* start , void* end , int);
void init_free();
void init_mm_reserve();

#endif