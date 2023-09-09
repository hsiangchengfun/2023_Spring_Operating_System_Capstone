#include "shell.h"
#include "uart.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "cpio.h"
#include "memory.h"
#include "timer.h"
#include "exception.h"
#include "math.h"
#include "mm.h"
#include "sched.h"
#include "syscall.h"
#include "peripherals/mailbox.h"
#include "fork.h"

#define MAX_BUFFER_SIZE 256

static char command[MAX_BUFFER_SIZE];
static int shared = 1;
static void* to_free[10] = {(void*)MEM_START};
static void* to_freec[4] = {(void*)MEM_START};


void readcmd();
void parsecmd();


void shell() 
{
    while (1) {
        printf("\r\nAnonymousELF@Rpi3B+ >>> ");
        readcmd();
        parsecmd();
    }
}

void foo() {
    for(int i = 0; i < 10; ++i) {
        printf("Thread id: %d loop %d\n", current->id, i);
        delay(1000000);
        schedule();
        // printf("=================================\n");
    }

    current->state = TASK_ZOMBIE;
    while(1);
}

void user_foo(){


    printf("Fork Test , pid : %d\n",getpid());


    // int pid = fork();
    uint32_t cnt = 1,ret=0;

    if((ret=fork()) == 0){ //pid == 0 => child
        printf("--Child Process--\n");
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first  child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;


        if ((ret = fork() )!= 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first  child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                ++cnt;
            }
        }
        exit();
        
    }else{ //pid > 0 => parent
        printf("parent here, pid %d, child %d\n", getpid(), ret);

    }
    

    exit();
}


void start_video() {
    
    // start sycall.img user process
    // preempt enable

    //first find syscall.img
    struct cpio_newc_header *header;
    unsigned int filesize;
    unsigned int namesize;
    unsigned int offset;
    char *filename;
    void *code_loc;

    header = DEVTREE_CPIO_BASE;
    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (strncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (strncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = atoi(header->c_namesize, 8);
        
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;

        filesize = atoi(header->c_filesize, 8);

        if (strncmp(filename, "syscall.img", namesize) == 0) {
            code_loc = ((void*)header) + offset;
            break;
        }

        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }
    printf("syscall.img found in cpio at location 0x%x.\n", code_loc);
    printf("syscall.img has size of %d bytes.\n", (int)filesize);
    
    
    void *move_loc = malloc(filesize + 4096); 
    // an extra page for bss just in case
    if(move_loc == NULL) return;
    //move syscall.imag to an allocated area
    for (int i=0; i < filesize; i++) {
        ((char*)move_loc)[i] = ((char*)code_loc)[i];
    }

    preempt_disable();
    
    //set current process STOP
    current->state = TASK_STOPPED;
    unsigned long long tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1; // make el0 don't trap  ( bit[0] )
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    //create a thread to execute syscall.img
    copy_process(PF_KTHREAD, (unsigned long)&new_user_process, (unsigned long)move_loc, 0);
    preempt_enable();

}

void readcmd()
{
    uint32_t idx = 0;
    char c = '\0';
    
    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (idx < MAX_BUFFER_SIZE) command[idx] = '\0';
            else command[MAX_BUFFER_SIZE-1] = '\0';
            
            break;
        } else {
            uart_send(c);
            command[idx++] = c;
        } 
    }

}

void parsecmd()
{

    if (strcmp(command, "\0") == 0){
        uart_send_string("\n");
    }else if(strcmp(command,"help")==0){
        printf("\r\nhelp            : print this help menu\r\n");
        printf("hello           : print Hello World!\r\n");
        printf("mailbox         : print Hardware Information\r\n");
        printf("ls              : list files existed in this folder\r\n");
        printf("cat             : print specific file content\r\n");
        printf("test_malloc     : test malloc function\r\n");
        printf("reboot          : reboot the device\r\n");
        printf("execute         : run user program\r\n");
        printf("async           : enable async uart\r\n");
        printf("boottime        : check boot time\r\n");
        printf("multiplex       : test time multiplexing\r\n");
        printf("thread_test     : test thread\r\n");
        printf("to_user         : test to user\r\n");
        printf("vdo             : play video\r\n");

    }else if (strcmp(command, "hello") == 0){
        uart_send_string("Hello World!\n");
    }
    else if (strcmp(command, "mailbox") == 0){
        get_board_revision();
        get_memory_info();
    }
    else if (strcmp(command, "reboot") == 0){
        uart_send_string("Rebooting...\n");
        reset(100);
    }
    else if (strcmp(command, "ls") == 0){
        cpio_ls();
    }
    else if (strcmp(command, "cat") == 0){
        cpio_cat();
    }
    else if (strcmp(command, "test_malloc") == 0) {
        char *str = simple_malloc(21);
        str="test malloc success";
        str[20] = '\0';
        uart_send_string(str);
        uart_send('\n');
    }
    else if (strcmp(command, "execute") == 0) {
        
        cpio_exec();
    }else if(strcmp(command,"malloc")==0){
        for (int i=0; i<10; i++) {
            printf("%d ",i);
            printf("th time\r\n");
            if(i < 5) to_free[i] = malloc(4095);
            else to_free[i] = malloc(4096*pow(2, i-4));
            
            
        }
        
        printf("Last test 4096*64 malloc\r\n");
        malloc(4096*64);
        
        
    }else if(strcmp(command,"free")==0){
        for (int i=0; i<10; i++) {
            free(to_free[i]);
        } 
        
    }
    else if (strcmp(command, "thread_test") == 0) {
    
        for (int i=0; i<10; i++) {
            copy_process(PF_KTHREAD, (unsigned long)&foo, 0, 0);
        }
    
    }
    else if (strcmp(command, "to_user") == 0) {
        copy_process(PF_KTHREAD, (unsigned long)&new_user_process, (unsigned long)&user_foo, 0);
    }
    else if (strcmp(command, "vdo") == 0) {
        start_video();
    }else{
        uart_send_string("Command not found!\r\n");
    }
        

}




