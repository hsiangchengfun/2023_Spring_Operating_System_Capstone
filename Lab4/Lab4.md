# Lab4


### **Structure**
```c
struct frame{
    uint32_t index;
    struct frame *prev , *next;//Link frame blocks
    int order;//record the merged order it belongs to
    int stat;
};

struct node{
    struct  node *prev, *next;
};

struct memory_pool{
    uint32_t frame_used; // num of frame used
    uint32_t chunk_used; // num of chunk used
    uint32_t chunk_size; // record size of chunk
    uint32_t chunk_per_frame;  
    uint32_t chunk_offset_each_frame;
    struct node* unallocated_ptr; 
    void *frame_base_addrs[MAX_POOL_PAGES]; //
};

struct reserved_node
{
	struct reserved_node* next;
    struct int_pair pair; //a pair to record reserve region's start and end
};
```

### **Functions**

1. malloc
    * Get the over and nearest frame size(2 power) of input size (its buddy size)
    * Traverse all the frame levels, if can't find the input size then find bigger level
    * If use larger block then cut it first
    * Else => target_level equal to size_level => directly allocate
        * Set the frame level array to link the frame used and set attributes


2. free_memory
    * Iterate all orders
    * Get buddy_index by **XOR** order power of 2 ( this way can get its "same 2's bit friend") 
    * If the frame is allocable then merge buddy_index and the input index
        * Set some linking relationship and attributes
        * Choose the larger one to be its merged index
    * Else set it **Allocable** and initialize that frame to be the head of the frame level array of corresponding size

3. init_pool
    * initialize attributes in pool
    * Such as size , used ...

4. get_chunk
    * Give a rounded size for input request
    * Iterate all the pools , if found size zero then initialize that pool
    * Else if match then return the index

5. dynamic_alloc
    * First get the chunk
    * If the pool's unallocated pointer is not NULL then allocate it from free frame list
    * Else if exceed limit of pool valumn then return error
    * Else if the pointer is NULL and exceed pool's frame size then new a frame for it
    * Else allocate it from the existed corresponding pool 

6. dynamic_free
    * We know Objects from the same page frame have a common prefix address. 
    * First get prefix address by **and** the address with ~0xFFF (discard last 12 bits)
    * Iterate all pools to find the corresponding pool for the base address
    * Set that found pool's pointer to the real address
    * Set the other attributs


7. init_mm_reserve
    * Call reserved_memory for specific address regions
        * Two regions reserved
            * Kernel region
            * Devicetree region
    * Initialize all frame
    * iterate all frames and mark the frame in reserved list as **RESERVED**
    * pass to init_free

8. init_free
    * Go through all frames and skip the frame that are **RESERVED** or **CONTINUOUS**
    * For the frame not skipped , use a for loop of all orders to merge them all
    * Same as free_memory


9. reserved_memory
    * Record a pair ( start , end ) address for reserved list