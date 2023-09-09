#include "mm.h"
#include "utils.h"
#include "uart.h"
#include "mem.h"


static uint32_t max_alloc_size = 0;
static uint32_t n_frame = (MEM_END - MEM_START) / FRAME_SIZE;

extern char __kernel_end;
static char * __kernel_end_ptr = &__kernel_end;
static struct frame frame_array[(MEM_END-MEM_START) / FRAME_SIZE];
//all frames
static struct frame *frame_level_list[MAX_LEVEL] = {NULL};
//each index means number of that level
static struct memory_pool pool_array[MAX_POOLS] = { 0,0,0,0,0,NULL,{NULL} };
static struct reserved_node reserved_list[2]={NULL,NULL};


extern void *DEVICETREE_CPIO_BASE;
extern void* DEVICETREE_CPIO_END;


void reserved_memory(void* start , void* end,int ind){

    // struct reserved_node tmp;
    // tmp.pair.fst = (int)start;
    // tmp.pair.sec = (int)end;
    

    // tmp.next = reserved_list;
    reserved_list[ind].pair.fst = (int)start;
    reserved_list[ind].pair.sec = (int)end;
    // struct reserved_node* test=reserved_list;
    // while(test!=NULL){
    print_string("reserve ");
    print_h(reserved_list[ind].pair.fst);
    print_string(" ");
    print_h(reserved_list[ind].pair.sec);
    print_string("\r\n");
    //     test=test->next;
    // }
    

}


void init_mm(){

    n_frame = (MEM_END - MEM_START) / FRAME_SIZE;
    uint32_t max_group = (uint32_t)pow(2,MAX_LEVEL-1);
    print_string("[System Message] Frame array start address ");
    print_h(frame_array);
    print_string("\r\n");

    for(uint32_t i = 0; i < n_frame; i++){
            
        frame_array[i].index = i;
        if(i % max_group == 0){
            frame_array[i].prev = &frame_array[i - max_group];
            frame_array[i].next = &frame_array[i + max_group];
            frame_array[i].order = MAX_LEVEL - 1;
            frame_array[i].stat = ALLOCABLE;
        }else{
            frame_array[i].prev = NULL;
            frame_array[i].next = NULL;
            frame_array[i].order = LARGER_CONTI;
            frame_array[i].stat = LARGER_CONTI;
        }
        
    }
    frame_array[0].prev = NULL;
    frame_array[n_frame - max_group].next = NULL;

    for(uint32_t i = 0 ; i < MAX_LEVEL ; i++ ){
        frame_level_list[i] = NULL;
    }

    frame_level_list[MAX_LEVEL - 1] = &frame_array[0];
    max_alloc_size = FRAME_SIZE * pow(2,MAX_LEVEL - 1);

    print_string("Max allocate size is");
    print_num(max_alloc_size);
    print_string("\r\n");
    print_string("n_frame is");
    print_num(n_frame);
    print_string("\r\n");

}


void *malloc(uint32_t size){
    if(size > max_alloc_size){
        print_string("[Error] Exceed Max Allocable Size\r\n");
        return NULL;
    } 

    int size_level = 0;
    for(int i = FRAME_SIZE ; i < size ;i *= 2 ){
        size_level ++;
    } 
    // get the over and nearest frame size(2 power) of input size


    int target_level;
    for(target_level = size_level ; target_level < MAX_LEVEL;target_level++){
        if(frame_level_list[target_level] != NULL)break;
    }
    // traverse all the frame levels 
    // if can't find then find bigger level

    //if use larger block then cut it first
    while(target_level > size_level){
        
        struct frame *tmp1 = frame_level_list[target_level];
        uint32_t offset = (uint32_t)pow(2,target_level - 1);
        struct frame *tmp2 = &frame_array[tmp1->index + offset];

        
        
        frame_level_list[target_level] = tmp1->next;
        frame_level_list[target_level]->prev = NULL;

        print_string("[System Message] Cut frame of level ");
        print_num(target_level + 1);
        print_string("\r\n");

        print_string("[System Message] Frame Level List [target] now start from ");
        print_h(frame_level_list[target_level]);
        print_string(" with size ");
        print_num(pow(2,frame_level_list[target_level]->order));
        print_string("\r\n");

        

        tmp1->next = tmp2;
        tmp1->prev = NULL;
        tmp1->stat = ALLOCABLE;
        tmp1->order -= 1 ;

        tmp2->prev = tmp1;
        tmp2->next = NULL;
        tmp2->stat = ALLOCABLE;
        tmp2->order = tmp1->order;


        print_string("[System Message] Tmp1 at ");
        print_h(tmp1);
        print_string(" with size ");
        print_num(pow(2,tmp1->order));
        print_string("\r\n");

        print_string("[System Message] Tmp2 at ");
        print_h(tmp2);
        print_string(" with size ");
        print_num(pow(2,tmp2->order));
        print_string("\r\n");



        target_level--;

        //link to origin's next 
        tmp2->next = frame_level_list[target_level];
        
        if(frame_level_list[target_level] != NULL){
            frame_level_list[target_level]->prev = tmp2;
        }
        tmp2->next = frame_level_list[target_level];
        frame_level_list[target_level] = tmp1;

    }


    //if target_level equal to size_level
    //directly allocate
    struct frame *tmp = frame_level_list[size_level];
    frame_level_list[size_level] = tmp->next;
    frame_level_list[size_level]->prev = NULL;

    tmp->stat = ALLOCATED;
    tmp->prev = NULL;
    tmp->next = NULL;

    print_string("[System Message] Allocated at : ");
    print_h(MEM_START + FRAME_SIZE * (tmp->index));
    print_string(" to : ");
    print_h(MEM_START + FRAME_SIZE * (tmp->index) + size);
    print_string("\r\n");
    


    return (void*)(MEM_START + FRAME_SIZE * (tmp->index));

}

void free_memory(void* addr){

    uint32_t index = ((uint64_t)addr - MEM_START ) / FRAME_SIZE;
    struct frame* target = &frame_array[index];

    if(target->stat != ALLOCATED){
        print_string("[ERROR] Invalid free address\r\n");
        return ;
    }
    
    print_string("==========================================\r\n");    
    print_string("[System Message] Free address ");
    print_h(addr);
    print_string(" frame num ");
    print_num(index);
    print_string("\r\n");

    



    for(int i = target->order;i < MAX_LEVEL;i++){

        uint32_t buddy_index = index ^ (uint32_t)pow(2,i);
        //use XOR to get its "same 2's bit friend"

        struct frame* buddy_frame = &frame_array[buddy_index];  


        if( i < MAX_LEVEL - 1 && i == buddy_frame->order && buddy_frame->stat == ALLOCABLE){

            print_string("[System Message] Merge frame[");
            print_num((int)buddy_index);
            print_string("] and frame[");
            print_num((int)index);
            print_string("] to order ");
            print_num(i);
            print_string("\r\n");

            if(buddy_frame->prev != NULL && buddy_frame->next != NULL){
                buddy_frame->prev->next = buddy_frame->next;
                buddy_frame->next->prev = buddy_frame->prev;

            }else if(buddy_frame->prev != NULL){
                buddy_frame->prev->next = NULL;

            }else if(buddy_frame->next != NULL){
                buddy_frame->next->prev = NULL;
            }

            if(buddy_frame->prev == NULL){
                frame_level_list[buddy_frame->order] = buddy_frame->next;
            }

            target->order = LARGER_CONTI;
            target->stat = LARGER_CONTI;
            buddy_frame->order = LARGER_CONTI;
            buddy_frame->order = LARGER_CONTI;
            buddy_frame->prev = NULL;
            buddy_frame->next = NULL;

            index = (target->index > buddy_frame->index)? buddy_frame->index : index;
            target = (target->index > buddy_frame->index)? buddy_frame : target;


        }else{
            target->order = i;
            target->stat = ALLOCABLE;
            target->next = frame_level_list[target->order ];
            target->prev = NULL;

            if(frame_level_list[target->order ] != NULL){
                frame_level_list[target->order ]->prev = target;
            }
            frame_level_list[target->order ] = target;
            print_string("[System Message] Frame ");
            print_num(target->index);
            print_string(" free to be haed of level ");
            print_num(target->order );
            print_string("\r\n");        
            break;
        }
    }  


    print_string("[System Message] Free Done\r\n");


}

void check_state(){
    print_string("==========================================\r\n");    
    for(int i = 0;i < MAX_LEVEL;i++){
        if(frame_level_list[i]!=NULL){
            print_string("Head of frame level ");
            print_num(i);
            print_string(" is array[");
            print_num(frame_level_list[i]->index);
            print_string("]\r\n");
        }else{
            print_string("Head of frame level ");
            print_num(i);
            print_string(" is Null\r\n");
        }
    }    

    for(int i=0;i<64;i+=pow(2,frame_array[i].order)){
        print_string("==========   ");
    }
    print_string("\r\n");
    for(int i=0;i<64;i+=pow(2,frame_array[i].order)){
        print_string("= 4k* ");
        print_num(pow(2,frame_array[i].order));
        
        if(pow(2,frame_array[i].order)>9)print_string(" =   ");
        else print_string("  =   ");
    }
    print_string("\r\n");
    for(int i=0;i<64;i+=pow(2,frame_array[i].order)){
        print_string("==========   ");
    }
    print_string("\r\n");


}


void init_pool( struct memory_pool* pool , uint32_t size){
    pool->chunk_size = size;
    pool->chunk_per_frame = FRAME_SIZE / size;
    pool->chunk_used = 0;
    pool->frame_used = 0;
    pool->unallocated_ptr = NULL;

}


int get_chunk(uint32_t size){

    int rounded_size = 0;
    if(size <= 8)rounded_size = 8;
    else rounded_size = (size % 4 != 0)? 4 * ((size + 4) / 4): size ;

    if(rounded_size > FRAME_SIZE) print_string("[ERROR] Exceed Frame Limit!!!\r\n");

    for(int i = 0; i < MAX_POOL_PAGES ;i++){
        if(pool_array[i].chunk_size == 0){
            init_pool( &pool_array[i], rounded_size);
            return i;
        }else if(rounded_size == pool_array[i].chunk_size){
            return i;
        }
    }
    return -1;

}


void *dynamic_alloc(uint32_t size){

    int pool_index = get_chunk(size);
    if(pool_index < 0) return NULL;


    struct memory_pool* pool = &pool_array[pool_index];

    if( pool->unallocated_ptr != NULL ){
        
        void* target = (void*)pool->unallocated_ptr;
        pool->unallocated_ptr = pool->unallocated_ptr->next;

        print_string("[System Message] Allocate from free list at ");
        print_h(target);
        print_string("\r\n");
        return target;
        
    }
    
    if( pool->chunk_used >= pool->chunk_per_frame * MAX_POOL_PAGES ){
        print_string("[ERROR] Exceed Pool Max Volume!!!\r\n");
        return NULL;
    }

    

    //out of page
    //new a page for the pool
    if( pool->chunk_used >= pool->frame_used * pool->chunk_per_frame ){
        pool->frame_base_addrs[pool->frame_used] = malloc(FRAME_SIZE);
        pool->frame_used ++;
        pool->chunk_offset_each_frame = 0;
        print_string("[System Message] Exceed frame size, newing a frame\r\n");

    }

    void* target = pool->frame_base_addrs[pool->frame_used - 1] + pool->chunk_size * pool->chunk_offset_each_frame;
    print_string("[System Message] Allocate from pool at ");
    print_h(target);
    print_string("\r\n");

    pool->chunk_offset_each_frame++;
    pool->chunk_used++;
    return target;

}


void dynamic_free(void* addr){
    //return one chunk to the caller. 
    //Objects from the same page frame have a common prefix address. 
    void* prefix_addr = (void*)((uint64_t)addr & ~0xFFF);//12 bits
    // 4k (2^12)

    print_string("free addr ");
    print_h((uint32_t)addr);                        
    print_string("\r\n");

    int target = -1;
    for(int i=0;i<MAX_POOLS;i++){
        for(int j=0;j<pool_array[i].frame_used;j++){
            void* base_addr = pool_array[i].frame_base_addrs[j];
            if( prefix_addr ==  base_addr){
                target = i;

            }
        }
    }
    if(target==-1){
        print_string("[ERROR] Free chunk Not Found\r\n");
        return;
    }


    struct memory_pool* pool = &pool_array[target];
    struct node* tmp = pool->unallocated_ptr;
    pool->unallocated_ptr = (struct node*)addr;
    pool->unallocated_ptr->next = tmp;
    pool->chunk_used -- ;
    print_string("[System Message] Free chunk from pool[");
    print_num(target);
    print_string("]\r\n");
    
    

}

void init_free(){

    for(int i=0; i < MAX_LEVEL ;i++){
        frame_level_list[i] = NULL;
    } 

    for( uint32_t frame = 0; frame < n_frame ;frame++ ){
        struct frame* target = &frame_array[frame];
        uint32_t index = frame;
        

        if(target->stat == RESERVED || target->stat == LARGER_CONTI){
            continue;
        }

        for(int i = target->order;i < MAX_LEVEL;i++){

            uint32_t buddy_index = index ^ (uint32_t)pow(2,i);
            //use XOR to get its "same 2's bit friend"

            struct frame* buddy_frame = &frame_array[buddy_index];  


            if( i < MAX_LEVEL - 1 && i == buddy_frame->order && buddy_frame->stat == ALLOCABLE){

                if(buddy_frame->prev != NULL && buddy_frame->next != NULL){
                    buddy_frame->prev->next = buddy_frame->next;
                    buddy_frame->next->prev = buddy_frame->prev;

                }else if(buddy_frame->prev != NULL){
                    buddy_frame->prev->next = NULL;

                }else if(buddy_frame->next != NULL){
                    buddy_frame->next->prev = NULL;
                }

                if(buddy_frame->prev == NULL){
                    frame_level_list[buddy_frame->order] = buddy_frame->next;
                }

                target->order = LARGER_CONTI;
                target->stat = LARGER_CONTI;
                buddy_frame->order = LARGER_CONTI;
                buddy_frame->order = LARGER_CONTI;
                buddy_frame->prev = NULL;
                buddy_frame->next = NULL;

                index = (target->index > buddy_frame->index)? buddy_frame->index : index;
                target = (target->index > buddy_frame->index)? buddy_frame : target;


            }else{
                target->order = i;
                target->stat = ALLOCABLE;
                target->next = frame_level_list[target->order ];
                target->prev = NULL;

                if(frame_level_list[target->order ] != NULL){
                    frame_level_list[target->order ]->prev = target;
                }
                frame_level_list[target->order ] = target;
                       
                break;
            }
        }  

    }


}

void init_mm_reserve(){
    max_alloc_size = FRAME_SIZE * pow(2,MAX_LEVEL - 1);
    print_string("k end ");
    print_h(*__kernel_end_ptr);
    print_string("\r\n");

    reserved_memory((void*)0x00000000,*__kernel_end_ptr, 0);
    reserved_memory((void*)DEVICETREE_CPIO_BASE, (void*)DEVICETREE_CPIO_END , 1);
    // return;

    for(uint32_t i = 0 ;i < n_frame ;i++){
        frame_array[i].prev = NULL;
        frame_array[i].next = NULL;
        frame_array[i].index = i;
        frame_array[i].order = 0;
        frame_array[i].stat = ALLOCABLE;
    }
    int reserve_cnt=2;
    int reserve_addr=0;
    // while(reserved_list != NULL){
    for(int i=0;i<reserve_cnt;i++){
        for(uint32_t i = 0; i < n_frame ;i++){

            void *addr = (void*)MEM_START + FRAME_SIZE * i;
            
            if( addr >= (void*)reserved_list->pair.sec )break;

            if( addr >= (void*)reserved_list->pair.fst && addr < (void*)reserved_list->pair.sec ){
                frame_array[i].stat = RESERVED;
            }
            
        }
        // print_string("[System Message] Reserve ");
        // print_num(reserve_addr);
        // reserve_addr++;
        // print_string("\r\n");
        // reserved_list = reserved_list->next;
    }

    init_free();
    print_string("[System Message] Init Reserve Done\r\n");


}
