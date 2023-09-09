#include "syscall.h"


/*系統調用號通過w8寄存器傳遞的原因是，
寄存器x0到x7被保留給系統調用傳遞參數使用。
如果有多個參數，這些參數將按順序加載到寄存器x0到x7中，
然後使用svc指令進行系統調用。這個過程在glibc中進行封裝，
因此在Linux上的使用者無法察覺到svc指令的調用。*/

.global getpid
getpid:
    mov w8, #SYS_GETPID_NUM
    svc #0
    ret

.global uart_read
uart_read:
    mov w8, #SYS_UARTREAD_NUM
    svc #0
    ret

.global uart_write
uart_write:
    mov w8, #SYS_UARTWRITE_NUM
    svc #0
    ret

.global exec
exec:
    mov w8, #SYS_EXEC_NUM
    svc #0
    ret

.global fork
fork:
    mov w8, #SYS_FORK_NUM
    svc #0
    ret

.global exit
exit:
    mov w8, #SYS_EXIT_NUM
    svc #0
    ret

.global mbox_call
mbox_call:
    mov w8, #SYS_MBOXCALL_NUM
    svc #0
    ret

.global kill
kill:
    mov w8, #SYS_KILL_NUM
    svc #0
    ret
