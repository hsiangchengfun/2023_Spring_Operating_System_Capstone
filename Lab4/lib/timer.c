#include "mem.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"
#include "exception.h"

static struct event *timers = 0;

void infinite() {
    while (1);
}


uint64_t get_core_frequency() { //ok
    uint64_t frequency;
    asm volatile("mrs %0, cntfrq_el0\r\n" :"=r"(frequency));
    return frequency;
}

uint64_t get_core_count() { //ok
    uint64_t count;
    asm volatile("mrs %0, cntpct_el0\r\n" :"=r"(count));
    return count;
}



void core_timer_enable() { //ok
    asm volatile( "mov    x0, 1\r\n\t" );
    asm volatile( "msr    cntp_ctl_el0, x0\r\n\t");     // enable
    asm volatile( "mov    x0, 2\r\n\t"); // set expired time
    asm volatile( "ldr    x1, =0x40000040\r\n\t");      // CORE0_TIMER_IRQ_CTRL
    asm volatile( "str    w0, [x1]\r\n\t");             // unmask timer interrupt

}

void core_timer_disable() { //ok
    asm volatile( "mov    x0, 0\r\n\t" );
    asm volatile( "msr    cntp_ctl_el0, x0\r\n\t");     // disable
    asm volatile( "mov    x0, 2\r\n\t"); // set expired time
    asm volatile( "ldr    x1, =0x40000040\r\n\t");      // CORE0_TIMER_IRQ_CTRL
    asm volatile( "str    w0, [x1]\r\n\t");             // unmask timer interrupt

}

void add_core_timer(void (*func)(void*), void *args, uint32_t time) {
    struct event *timer = (struct event*) simple_alloc(sizeof(struct event));

    timer->args     = args;
    timer->expired_time  = get_core_count() + time;
    timer->callback = func;
    timer->next     = 0;

    int update = 0;

    if (!timers) {
        timers = timer; update = 1;
    } else if (timers->expired_time > timer->expired_time) {
        //sorting timers and new timer is the bigger then directly put back
        timer->next = timers;
        timers = timer;
        update = 1;
    } else {
        //if newest not largest then sort and put
        struct event *current = timers;
        while (current->next && current->next->expired_time < timer->expired_time) {
            current = current->next;
        }

        timer->next = current->next;
        current->next = timer;
    }

    if (update) {
        set_core_timer(time);
    }
}

void set_core_timer(uint32_t time) { //ok
    asm volatile("msr cntp_tval_el0, %0\r\n" :"=r"(time)); 
    //set timeout => wait 2*freq times
}

void remove_core_timer() {
    struct event *timer = timers;
    timer->callback(timer->args);
    timers = timers->next;

    if (!timers) {
        core_timer_disable();
    } else {
        uint32_t count = get_core_count();
        set_core_timer(timers->expired_time - count);
    }
}

void print_boot_time() { //ok
    mini_uart_puts("Core Timer Interrupt!\r\n");
    

    uint64_t frequency = get_core_frequency();
    uint64_t current   = get_core_count();

    mini_uart_puts("Time after booting: ");
    print_num(current / frequency);
    mini_uart_puts("(secs)\r\n\r\n");

    set_core_timer(2 * get_core_frequency());
}

void print_core_timer_message(void *args) {
    mini_uart_puts((char*) args);
    print_boot_time();
}

