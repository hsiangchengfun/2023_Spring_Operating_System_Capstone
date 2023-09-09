#include "shell.h"
#include "mini_uart.h"
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
extern void *DEVTREE_CPIO_BASE;

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
    printf("devtree base: 0x%x\n", DEVTREE_CPIO_BASE);

    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = atoi(header->c_namesize, 8);
        
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;

        filesize = atoi(header->c_filesize, 8);

        if (stringncmp(filename, "vm.img", namesize) == 0) {
            code_loc = ((void*)header) + offset;
            break;
        }

        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }
    printf("vm.img found in cpio at location 0x%x.\n", code_loc);
    printf("vm.img has size of %d bytes.\n", (int)filesize);
    

    int err = move_to_user_mode((unsigned long)code_loc, filesize, 0);
    if (err<0){
        printf("failed to start video program\n");
    } 
        

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
        printf("vdo             : run video vm program\r\n");

        

    }else if (strcmp(command, "hello") == 0){
        uart_send_string("Hello World!\n");
    }
    else if (strcmp(command, "mailbox") == 0){
        get_board_revision();
        get_arm_memory();
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
        
    }else if (strcmp(command, "vdo") == 0) {
        preempt_disable();
    
        //set current process STOP
        current->state = TASK_STOPPED;
        unsigned long long tmp;
        asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
        tmp |= 1;
        printf("=====%ld=====",tmp);
        asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
        //fork a thread to execute vm.img
        copy_process(PF_KTHREAD, (unsigned long)&start_video,0);

        preempt_enable();

    }else{
        uart_send_string("Command not found!\r\n");
    }
        

}




