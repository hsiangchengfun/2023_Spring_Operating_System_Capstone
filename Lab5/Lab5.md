# Lab5

<hr>

### **sched.h / sched.c**
* Implement the schedule() API. When the current thread calls this API, the scheduler picks the next thread from the run queue. 
* In this lab, your scheduler should at least be able to schedule the threads of the same priority in a round-robin manner.

1. Use a **current** task_struct to present curent task
2. Another task_struct array record all the tasks 
3. 


#### **Structs**
```c
struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

struct task_struct { //
	struct cpu_context cpu_context;
	long state;	        // Task state Running / Zombie or else
	long counter;		//For Round Robin Manner
	long priority;
	long preempt_count; // If can do preemption of not
	unsigned long stack;
	unsigned long flags;//Kernel / User
    long id;            //task id (pid)
};
```



#### **Functions**
1. schedule<br>
    * Set current task's counter 0
    * Disable preempt
    * Use a while loop untill scheduled a task
    * Iterate all the tasks to find the running one who also has largest counter
    * If no task scheduled then update the counter of tasks
    * When all done then enable preemption


2. switch_to<br>
    * Pass current task to cpu_switch_to


3. cpu_switch_to ( In assembly )<br>
    * thinking


4. exit_process<br>
    * Set task state as ZOMBIE
    * Free the space of the current task's stack


5. kill_zombie<br>
    * Iterate all tasks
    * Find the ZOMBIE state taks and free them

6. preempt enable/disable <br>
    * Add / Minus preemption value (disable -> 0  ,  enable -> 1)


7. timer_tick<br>
    * If current task's counter > 0 or its preempt_count > 0 then don't reschedule and let it run
    * Else raise interrupt and reschedule



<hr>

### **Fork.h / Fork.c**

#### PSTATE
* M(mode): 現在處理器運行的模式
    * M[4]: 0表示現在是AArch64，1則表示現在是在AArch32 
    * M[3:0]: Current processor mode
    ```c
    #define PSR_MODE_EL0t	0x00000000
    #define PSR_MODE_EL1t	0x00000004
    #define PSR_MODE_EL1h	0x00000005
    #define PSR_MODE_EL2t	0x00000008
    #define PSR_MODE_EL2h	0x00000009
    #define PSR_MODE_EL3t	0x0000000c
    #define PSR_MODE_EL3h	0x0000000d
    ```

#### **Functions**
1. copy_process<br>
    * Disable preemption
    * Create a new task and a struct of regiters
    * If input is a **KERNEL Thread** then store funtion and register to task's context register
    * Else clone a **USER Thread** and copy all current registers, meanwhile set x0 to zero becuase x0 is used as return value of syscall
    * Move stack pointer to current stack's top
    * Set other attributes and return pid

2. move_to_user<br>
    * Create and initialize pt_registers
    * Set pc , PSTATE and stack pointer

3. new_user_process<br>
    * pass to move_to_user

4. task_pt_regs<br>
    * return the location of pt_registers
    ``` 
    |-----------TASK-----------|
    |          THREAD          |
    |-pt_regs-|----------------|
         |
         | -> sizeof( pt_regs )
    ```
#### **Structs**
```c
struct pt_regs {
    unsigned long regs[31];     //registers
    unsigned long sp;           //stack pointer
    unsigned long pc;           //program counter; 
    //kernel_exit will copy pc to the elr_el1 register, 
    //Making sure that we will return to the pc address after exception

    unsigned long pstate;//process state; 
    //This field will be copied to spsr_el1 by the kernel_exit and 
    //becomes the processor state after exception return is completed.
};
```
<hr>

### **Syscall.h / Syscall.c**

Syscall use w8 register to send value because x0-x7 are save to send syscall parameters , if multiple parameters exists , will be put into x0-x7 in order, then use **svc** command to call.


#### **Functions**
* Accomplish in assembly

```c
unsigned sys_uartread(char buf[], unsigned size); 
unsigned sys_uartwrite(const char buf[], unsigned size);
int sys_exec(const char *name, char *const argv[]);
int sys_fork();
void sys_exit(int status); 
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);
```

```assembly
#include "syscall.h"

.global getpid
getpid:
    mov w8, #SYS_GETPID_NUM
    svc #0
    ret

//according to syscall send its own number 
//use SVC cmd to raise exception

.global uart_read
uart_read:
    mov w8, #SYS_UARTREAD_NUM
    svc #0
    ret
...

```