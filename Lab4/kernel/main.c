#include "cpio.h"
#include "uart.h"
#include "mem.h"
#include "utils.h"
#include "mm.h"
#include "reboot.h"
#include "mailbox.h"
#include "timer.h"
#include "exception.h"
#include "devtree.h"
#include "../include/sched.h"




#define MAX_LEN 1024


uint64_t initrd_addr;

int readcmd(char *buf, int len);
static void* to_free[10] = {(void*)MEM_START};
static void* to_freec[4] = {(void*)MEM_START};

void shell();




int main(void) {
    mini_uart_init();

    fdt_traverse(initramfs_callback);

    // enable_interrupt();
    // asm volatile("msr DAIFCLr, 0xf\r\n");
    // init_mm();
    init_mm_reserve();

    print_string("\r\nWelcome to AnonymousELF's shell\r\n");

    while (1) {
        shell();
    }

    return 0;
}



void shell(){
    

    char command[BUFSIZE];
    mini_uart_gets(command, BUFSIZE);

    if(strcmp(command,"help")==0){
        print_string("\r\nhelp            : print this help menu\r\n");
        print_string("hello           : print Hello World!\r\n");
        print_string("mailbox         : print Hardware Information\r\n");
        print_string("ls              : list files existed in this folder\r\n");
        print_string("cat             : print specific file content\r\n");
        print_string("test_malloc     : test malloc function\r\n");
        print_string("reboot          : reboot the device\r\n");
        print_string("execute         : run user program\r\n");
        print_string("async           : enable async uart\r\n");
        print_string("boottime        : check boot time\r\n");
        print_string("multiplex       : test time multiplexing\r\n");
        print_string("malloc          : do malloc\r\n");
        print_string("free            : free memory\r\n");
        print_string("dynamic_malloc  : dynamic malloc\r\n");
        print_string("dynamic_free    : free dynamic malloc\r\n");
        print_string("check           : check memory state\r\n");
        


    }else if(strcmp(command,"hello")==0){
        print_string("\r\nHello World!");
    
    }else if(strcmp(command,"mailbox")==0){
        print_string("\r\nMailbox info :\r\n");
        get_board_revision();
        get_memory_info();
    
    }else if(strcmp(command,"reboot")==0){
        print_string("\r\nRebooting ...\r\n");
        reset(200);

    }else if(strcmp(command,"ls")==0){
        cpio_ls();
        
    }else if(strcmp(command,"cat")==0){
        
        cpio_cat();
    
    }else if(strcmp(command,"test_malloc")==0){
        print_string("\r\nTest str1 = malloc(7) ; str1 = \"Success\"\r\n");
        char *str1 = (char *)simple_alloc(7);
        strncpy(str1,"Success",7);
        str1[7]=0;
        print_string("Answer str1 = ");
        print_string(str1);
        mini_uart_puts("\r\n");
        
        print_string("Answer str1 address = ");
        print_h((uint32_t)str1);
        mini_uart_puts("\r\n");
        
        print_string("\r\nTest str2 = malloc(30) ; str2 = \"Longer String Success Too\"\r\n");
        char *str2 = simple_alloc(30);
        strncpy(str2,"Longer String Success Too",30);
        str2[30]=0;
        print_string("Answer str2 = ");
        print_string(str2);
        mini_uart_puts("\r\n");
        print_string("Answer str2 address = ");
        print_h((uint32_t)str2);
        mini_uart_puts("\r\n");
    }else if(strcmp(command,"execute")==0){
        
        cpio_exe();
    
    }else if(strcmp(command,"boottime")==0){
        disable_interrupt();

        core_timer_enable();
        set_core_timer(2 * get_core_frequency());

        void (*location)(void) = infinite;

        asm volatile( "msr     elr_el1, %0\r\n\t" ::"r" (location) );
        asm volatile( "mov     x0, 0x340\r\n\t");
        asm volatile( "msr     spsr_el1, x0\r\n\t");
        asm volatile( "mov     x0, sp\r\n\t");
        asm volatile( "msr     sp_el0, x0\r\n\t");
        asm volatile("eret    \r\n\t");

        infinite();
    }else if (strcmp(command,"async")==0){
        enable_uart_interrupt();
        
        // delay(50000);

        char input[BUFSIZE];
        while(1){
            async_uart_puts("(async)>>> ");
            uint32_t input_len = async_uart_gets(input,BUFSIZE);
            if(input_len==0){
                async_uart_puts("\r\n");
                continue;
            }else{
                async_uart_puts(input);
                async_uart_puts("\r\n");
                if(strncmp(input,"exit",input_len)==0){
                    break;
                }
            }

        }

        // delay(50000);
        disable_uart_interrupt();


    }else if(strcmp(command,"multiplex")==0){
        disable_interrupt();
        char* msg1 = (char*)simple_alloc(16);
        char* msg2 = (char*)simple_alloc(16);
        char* msg3 = (char*)simple_alloc(16);

        msg1 = "message1\r\n";
        msg2 = "message2\r\n";
        msg3 = "message3\r\n";

        add_core_timer(print_core_timer_message,msg1,1*get_core_frequency());
        add_core_timer(print_core_timer_message,msg2,4*get_core_frequency());
        add_core_timer(print_core_timer_message,msg3,2*get_core_frequency());


        // enable_interrupt();
        asm volatile("msr DAIFCLr, 0xf\r\n");
        core_timer_enable();

        int enable = 1;

        while (enable) {
            asm volatile("mrs %0, cntp_ctl_el0\r\n" :"=r"(enable));
        }


    }else if(strcmp(command,"malloc")==0){
        for (int i=0; i<10; i++) {
            print_num(i);
            print_string("th time\r\n");
            if(i < 5) to_free[i] = malloc(4095);
            else to_free[i] = malloc(4096*pow(2, i-4));
            
            
        }
        
        print_string("Last test 4096*64 malloc\r\n");
        malloc(4096*64);
        
        
    }else if(strcmp(command,"free")==0){
        for (int i=0; i<10; i++) {
            free_memory(to_free[i]);
        } 
        
    }else if(strcmp(command, "dynamic_malloc") == 0){
        to_freec[0] = dynamic_alloc(8);
        print_string("\r\n");
        to_freec[1] = dynamic_alloc(8);
        print_string("\r\n");
        to_freec[2] = dynamic_alloc(96); 
        print_string("\r\n");
        to_freec[3] = dynamic_alloc(256);
        print_string("\r\n");

    }else if(strcmp(command, "dynamic_free") == 0){
        for (int i=0; i<4; i++) {
            dynamic_free(to_freec[i]);
        }
    }else if(strcmp(command, "all_free") == 0){
        for (int i=0; i<4; i++) {
            free_memory(to_freec[i]);
        }
    }else if(strcmp(command,"check")==0){
        check_state();
    }else{
        print_string("\nCommand not found");
    
    }


}


