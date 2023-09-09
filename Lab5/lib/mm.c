#include "mm.h"
#include "math.h"
#include "mailbox.h"
#include "memory.h"
#include "uart.h"
#include "utils.h"

static unsigned int n_frames = 0;
static unsigned int max_size = 0;
static struct frame* frame_list[MAX_ORDER] = {NULL};                
static struct frame frame_array[(MEM_END-MEM_START)/PAGE_SIZE];     
static struct dynamic_pool pools[MAX_POOLS] = { {ALLOCABLE, 0, 0, 0, 0, {NULL}, NULL} };
extern char __kernel_end;
static char *__kernel_end_ptr = &__kernel_end;
static struct reserved_node reserved_list[2]={NULL,NULL};
extern void *DEVTREE_CPIO_END;



void *malloc(unsigned int size){
    
    if(size > max_size){
        printf("[Error] Exceed Max Allocable Size\r\n");
        return NULL;
    } 

    int size_level = 0;
    for(int i = PAGE_SIZE ; i < size ;i *= 2 ){
        size_level ++;
    } 
    // get the over and nearest frame size(2 power) of input size


    int target_level;
    for(target_level = size_level ; target_level < MAX_ORDER;target_level++){
        if(frame_list[target_level] != NULL)break;
    }
    // traverse all the frame levels 
    // if can't find then find bigger level

    //if use larger block then cut it first
    while(target_level > size_level){
        
        struct frame *tmp1 = frame_list[target_level];
        uint32_t offset = (uint32_t)pow(2,target_level - 1);
        struct frame *tmp2 = &frame_array[tmp1->index + offset];

        
        
        frame_list[target_level] = tmp1->next;
        if(frame_list[target_level]!=NULL)frame_list[target_level]->prev = NULL;

        // printf("[System Message] Cut frame of level ");
        // printf("%d",target_level + 1);
        // printf("\r\n");

        // printf("[System Message] Frame Level List [target] now start from ");
        // printhex(frame_list[target_level]);
        // printf(" with size ");
        // printf("%d",pow(2,frame_list[target_level]->val));
        // printf("\r\n");

        

        tmp1->next = tmp2;
        tmp1->prev = NULL;
        tmp1->state = ALLOCABLE;
        tmp1->val -= 1 ;

        tmp2->prev = tmp1;
        tmp2->next = NULL;
        tmp2->state = ALLOCABLE;
        tmp2->val = tmp1->val;


        // printf("[System Message] Tmp1 at ");
        // printhex(tmp1);
        // printf(" with size ");
        // printf("%d",pow(2,tmp1->val));
        // printf("\r\n");

        // printf("[System Message] Tmp2 at ");
        // printhex(tmp2);
        // printf(" with size ");
        // printf("%d",pow(2,tmp2->val));
        // printf("\r\n");



        target_level--;

        //link to origin's next 
        tmp2->next = frame_list[target_level];
        
        if(frame_list[target_level] != NULL){
            frame_list[target_level]->prev = tmp2;
        }
        tmp2->next = frame_list[target_level];
        frame_list[target_level] = tmp1;

    }


    //if target_level equal to size_level
    //directly allocate
    struct frame *tmp = frame_list[size_level];
    frame_list[size_level] = tmp->next;
    if(frame_list[size_level]!=NULL)frame_list[size_level]->prev = NULL;

    tmp->state = ALLOCATED;
    tmp->prev = NULL;
    tmp->next = NULL;

    // printf("[System Message] Allocated at : ");
    // printhex(MEM_START + PAGE_SIZE * (tmp->index));
    // printf(" to : ");
    // printhex(MEM_START + PAGE_SIZE * (tmp->index) + size);
    // printf("\r\n");
    


    return (void*)(MEM_START + PAGE_SIZE * (tmp->index));
}

void free(void *addr){

    
    uint32_t index = ((uint64_t)addr - MEM_START ) / PAGE_SIZE;
    struct frame* target = &frame_array[index];

    if(target->state != ALLOCATED){
        printf("[ERROR] Invalid free address\r\n");
        return ;
    }
    
    printf("==========================================\r\n");    
    printf("[System Message] Free address ");
    printhex(addr);
    printf(" frame num ");
    printf("%d",index);
    printf("\r\n");

    



    for(int i = target->val;i < MAX_ORDER;i++){

        uint32_t buddy_index = index ^ (uint32_t)pow(2,i);
        //use XOR to get its "same 2's bit friend"

        struct frame* buddy_frame = &frame_array[buddy_index];  


        if( i < MAX_ORDER - 1 && i == buddy_frame->val && buddy_frame->state == ALLOCABLE){

            // printf("[System Message] Merge frame[");
            // printf("%d",(int)buddy_index);
            // printf("] and frame[");
            // printf("%d",(int)index);
            // printf("] to order ");
            // printf("%d",i);
            // printf("\r\n");

            if(buddy_frame->prev != NULL && buddy_frame->next != NULL){
                buddy_frame->prev->next = buddy_frame->next;
                buddy_frame->next->prev = buddy_frame->prev;

            }else if(buddy_frame->prev != NULL){
                buddy_frame->prev->next = NULL;

            }else if(buddy_frame->next != NULL){
                buddy_frame->next->prev = NULL;
            }

            if(buddy_frame->prev == NULL){
                frame_list[buddy_frame->val] = buddy_frame->next;
            }

            target->val = C_NALLOCABLE;
            target->state = C_NALLOCABLE;
            buddy_frame->val = C_NALLOCABLE;
            buddy_frame->val = C_NALLOCABLE;
            buddy_frame->prev = NULL;
            buddy_frame->next = NULL;

            index = (target->index > buddy_frame->index)? buddy_frame->index : index;
            target = (target->index > buddy_frame->index)? buddy_frame : target;


        }else{
            target->val = i;
            target->state = ALLOCABLE;
            target->next = frame_list[target->val ];
            target->prev = NULL;

            if(frame_list[target->val ] != NULL){
                frame_list[target->val ]->prev = target;
            }
            frame_list[target->val ] = target;
            // printf("[System Message] Frame ");
            // printf("%d",target->index);
            // printf(" free to be haed of level ");
            // printf("%d",target->val );
            // printf("\r\n");        
            break;
        }
    }  







}

void init_mm(){
    n_frames = (MEM_END-MEM_START) / PAGE_SIZE;       
    unsigned int mul=(unsigned int)pow(2,MAX_ORDER-1);
    printf("[info] Frame array start address 0x%x.\n", frame_array);
    for(unsigned int i=0;i<n_frames;i++){
        frame_array[i].index=i;
        if(i%mul==0){
            
            frame_array[i].val=MAX_ORDER-1;
            frame_array[i].state=ALLOCABLE;
            frame_array[i].prev=&frame_array[i-mul];
            frame_array[i].next=&frame_array[i+mul];
        }
        else{

            frame_array[i].val=C_NALLOCABLE;
            frame_array[i].state=C_NALLOCABLE;
            frame_array[i].prev=NULL;
            frame_array[i].next=NULL;
        }
    }
    frame_array[0].prev=NULL;               //list head
    frame_array[n_frames-mul].next=NULL;    //list tail

    for(int i=0;i<MAX_ORDER;i++){
        frame_list[i]=NULL;
    }

    frame_list[5]=&frame_array[0];      //initialize

    max_size=PAGE_SIZE * pow(2,MAX_ORDER-1);        
}

void init_pool(struct dynamic_pool* pool, unsigned int size){
    pool->chunk_size=size;
    pool->chunks_per_page=PAGE_SIZE/size;
    pool->chunks_allocated=0;
    pool->page_new_chunk_off=0;
    pool->pages_used = 0;
    pool->free_head = NULL;
}

int register_chunk(unsigned int size){

    unsigned int nsize=0;
    if(size<=8){
        nsize=8;
    }
    else{
        int remainder=size%4;
        if(remainder!=0){
            //round up
            nsize=(size/4 +1)*4;
        }
        else{
            nsize=size;
        }
    }

    if(nsize >= PAGE_SIZE){
        printf("[error] Normalized chunk size request leq page size.\n");
        return -1;
    }

    for(int i=0;i<MAX_POOLS;i++){
        if(pools[i].chunk_size==nsize){
            return i;
        }
        else if(pools[i].chunk_size == ALLOCABLE){
            init_pool(&pools[i], nsize);
            return i;
        }
    }

    return -1;
}




void *chunk_alloc(unsigned int size) {

    int pool_idx = register_chunk(size);

    if (pool_idx == -1) return NULL;
    
    struct dynamic_pool* pool = &pools[pool_idx];

    if (pool->free_head != NULL) {
        void *ret = (void*) pool->free_head;
        
        pool->free_head = pool->free_head->next;    
        //move pointer to next block
        return ret;
    }

    if (pool->chunks_allocated >= MAX_POOL_PAGES*pool->chunks_per_page) {

        return NULL;
    }
        

    if (pool->chunks_allocated >= pool->pages_used*pool->chunks_per_page) {
        pool->page_base_addrs[pool->pages_used] = malloc(PAGE_SIZE);
        
        pool->pages_used++;
        pool->page_new_chunk_off = 0;
    }

    void *ret = pool->page_base_addrs[pool->pages_used - 1] + 
                pool->chunk_size*pool->page_new_chunk_off;
    pool->page_new_chunk_off++;
    pool->chunks_allocated++;


    return ret;

}


void chunk_free(void *address) {

    int target = -1;

    void *prefix_addr = (void *)((unsigned long long)address & ~0xFFF); 

    for (unsigned int i=0; i<MAX_POOLS; i++) {
        for (unsigned int j=0; j<pools[i].pages_used; j++) {
            void *base = pools[i].page_base_addrs[j];
            if (base == prefix_addr)
                target = i; 
        }
    }
    struct dynamic_pool *pool = &pools[target];
    struct node* old_head = pool->free_head;
    pool->free_head = (struct node*) address;
    pool->free_head->next = old_head;
    pool->chunks_allocated--;

}

void memory_reserve(void* start, void* end,int ind) {
    if (ind >= MAX_RESERVABLE) {
        printf("[error] Max reservable locations already reached.\n");
        return;
    }
    reserved_list[ind].pair.fst = (int)start;
    reserved_list[ind].pair.sec = (int)end;
}

void init_mm_reserve(){

    max_size=PAGE_SIZE*pow(2,MAX_ORDER-1);
    n_frames=(MEM_END-MEM_START) / PAGE_SIZE;

    memory_reserve((void*)0x0, __kernel_end_ptr,0); 
    memory_reserve((void*)0x20000000, (void*)DEVTREE_CPIO_END,1); 

    //init
    for(unsigned int i=0;i<n_frames;i++){
        frame_array[i].index=i;
        frame_array[i].val=0;
        frame_array[i].state=ALLOCABLE;
        frame_array[i].prev=NULL;
        frame_array[i].next=NULL;
    }

    int j=0;
    for(unsigned int i=0;i<2;i++){
        for(;j<n_frames;j++){
            void *addr=(void*)MEM_START+PAGE_SIZE*j;
            //reserve the start/end address
            if( addr >= (void*)reserved_list->pair.fst && addr < (void*)reserved_list->pair.sec ){
                frame_array[j].state=RESERVED;
            }
            if( addr >= (void*)reserved_list->pair.sec )break;
        }
    }

    for(int i=0;i<MAX_ORDER;i++){
        frame_list[i]=NULL;
    }

    for(unsigned int n=0;n<n_frames;n++){

        struct frame* target = &frame_array[n];
        unsigned int idx=n;

        if(target->state == RESERVED) continue;
        if(target->state == C_NALLOCABLE) continue;

        for(int i=target->val;i<MAX_ORDER;i++){
            unsigned int buddy = idx ^ (unsigned int)pow(2, i);
            struct frame* fr_buddy = &frame_array[buddy];
            
            if(i<MAX_ORDER-1 && fr_buddy->state == ALLOCABLE && i==fr_buddy->val){

                if(fr_buddy->prev!=NULL){
                    fr_buddy->prev->next=fr_buddy->next;
                }
                else{
                    frame_list[fr_buddy->val]=fr_buddy->next;
                }

                if(fr_buddy->next!=NULL){
                    fr_buddy->next->prev=fr_buddy->prev;
                }

                fr_buddy->prev=NULL;
                fr_buddy->next=NULL;
                fr_buddy->val=C_NALLOCABLE;
                fr_buddy->state=C_NALLOCABLE;
                target->val=C_NALLOCABLE;
                target->state=C_NALLOCABLE;

                if(fr_buddy->index < target->index){
                    idx=fr_buddy->index;
                    target=fr_buddy;
                }
            }
            else{
                target->val = i;
                target->state = ALLOCABLE;
                target->prev = NULL;
                target->next = frame_list[i];
                if (frame_list[i] != NULL)
                    frame_list[i]->prev = target;
                frame_list[i] = target;
                break;
            }
        }
    }
}