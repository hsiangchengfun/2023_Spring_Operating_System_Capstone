SRCS = $(wildcard *.c)
ASMS = $(wildcard *.s)

OBJS = $(ASMS:%.s=%.o)
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -g
# -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -g
# Can't use mailbox  version => -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -g

all: clean user_prog.img  



%.o: %.s
	aarch64-linux-gnu-gcc $(CFLAGS) -c $^ -o $@


user_prog.img: $(OBJS)
	aarch64-linux-gnu-ld  -static   -o user_prog.elf  $(OBJS)
	aarch64-linux-gnu-objcopy user_prog.elf -O binary  $@
	cp ./user_prog.img ../rootfs/

clean:
	rm user_prog.* *.o >/dev/null 2>/dev/null || true
	clear




