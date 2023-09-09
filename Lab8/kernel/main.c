#include "mini_uart.h"
#include "shell.h"
#include "devtree.h"
#include "cpio.h"
#include "exception.h"
#include "mm.h"
#include "timer.h"
#include "fork.h"
#include "vfs.h"
#include "initramfs.h"
#include "sdhost.h"
#include "fat32.h"

void main(void)
{
	uart_init();
	devtree_getaddr();
	
	fdt_traverse(initramfs_callback);

	//init_mm();
	init_mm_reserve();
	timer_init();

	
	rootfs_init();
	initramfs_init();
	
	sd_init();
	fat32_init();

	enable_interrupt();

	printf("\r\nWelcome to AnonymousELF's shell\r\n");



	copy_process(PF_KTHREAD, (unsigned long)&shell, 0, 0);

	while (1) {
		/*When the idle thread is scheduled, it checks if there is any zombie thread. 
		If yes, it recycles them as follows.*/
		kill_zombies();
		schedule();
	}
}