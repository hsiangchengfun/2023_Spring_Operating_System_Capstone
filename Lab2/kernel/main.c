#include "uart.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"
#include "mem.h"
#include "devtree.h"

#define MAX_LEN 1024


uint64_t initrd_addr;

int readcmd(char *buf, int len);

void shell();

int main( )
{
    
    mini_uart_init();
    mem_init();

    print_string("\nWelcome to AnonymousELF's shell\n");
    // check_fdt_magic();

    uint32_t* t = 0x50000;
    print_char('\n');
    print_string("Dtb address 0x50000 contain is \n");
    print_h(*t);
    print_char('\n');
    fdt_traverse(initramfs_callback); //Use the API to get the address of initramfs instead of hardcoding it.

    while(1) {
        shell();
    }

    return 0;
}






void shell(){
    
    print_string("\nAnonymousELF@Rpi3B+ >>> ");

    char command[256];
    readcmd(command,256);
    static char buf[MAX_LEN];


    if(strcmp(command,"help")){
        print_string("\nhelp       : print this help menu\n");
        print_string("hello      : print Hello World!\n");
        print_string("mailbox    : print Hardware Information\n");
        print_string("ls         : list files existed in this folder\n");
        print_string("cat        : print specific file content\n");
        print_string("test malloc: test malloc function\n");
        print_string("reboot     : reboot the device");
    
    }else if(strcmp(command,"hello")){
        print_string("\nHello World!");
    
    }else if(strcmp(command,"mailbox")){
        print_string("\nMailbox info :\n");
        get_board_revision();
        get_memory_info();
    
    }else if(strcmp(command,"reboot")){
        print_string("\nRebooting ...\n");
        reset(200);

    }else if(strcmp(command,"ls")){
        cpio_ls();
        
    
    }else if(strcmp(command,"cat")){
        
        cpio_cat();
    
    }else if(strcmp(command,"test malloc")){
        print_string("\nTest str1 = malloc(7) ; str1 = \"Success\"\n");
        char *str1 = (char *)simple_malloc(7);
        strncpy(str1,"Success",7);
        str1[7]=0;
        print_string("Answer str1 = ");
        print_string(str1);
        print_char('\n');
        print_string("Answer str1 address = ");
        print_h(str1);
        print_char('\n');

        print_string("\nTest str2 = malloc(30) ; str2 = \"Longer String Success Too\"\n");
        char *str2 = simple_malloc(30);
        strncpy(str2,"Longer String Success Too",30);
        str2[30]=0;
        print_string("Answer str2 = ");
        print_string(str2);
        print_char('\n');
        print_string("Answer str2 address = ");
        print_h(str2);
        print_char('\n');
    }
    else{
        print_string("\nCommand not found");
    
    }


}
