CC      = aarch64-linux-gnu-gcc
LD      = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU    = qemu-system-aarch64

ASMFLAGS = -I ../include
CFLAGS   = -Wall -nostdlib -nostartfiles -ffreestanding -g -I ../include
# -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -g
# Can't use mailbox  version => -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -g


LIB_DIR   = ../lib
BUILD_DIR = ../build

C_FILES    = $(wildcard *.c)
ASM_FILES  = $(wildcard *.s)
OBJ_FILES  = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_FILES:%.s=$(BUILD_DIR)/%.o)

LIB_FILES    = $(wildcard $(LIB_DIR)/*.c)
LIBOBJ_FILES = $(LIB_FILES:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)

CPIO = ../initramfs.cpio
DTB  = ../bcm2710-rpi-3-b-plus.dtb

ELF   = kernel8.elf
PROGS = kernel8.img

all: $(PROGS) size


size: kernel8.img
	../make_size.sh ./kernel8.img

$(BUILD_DIR)/%.o: %.s
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(CC) $(ASMFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(ELF): $(OBJ_FILES) $(LIBOBJ_FILES) linker.ld
	$(LD) -T linker.ld -o $@ $(OBJ_FILES) $(LIBOBJ_FILES)

$(PROGS): $(BUILD_DIR)/$(ELF)
	$(OBJCOPY) -O binary $< $@

run: $(PROGS)
	$(QEMU) -M raspi3b -kernel $< -display none -serial null -serial stdio

cpio: $(PROGS)
	$(QEMU) -M raspi3b -kernel $< -display none -serial null -serial stdio -initrd $(CPIO)

dtb: $(PROGS)
	$(QEMU) -M raspi3b -kernel $< -display none -serial null -serial stdio -initrd $(CPIO) -dtb $(DTB)

debug: $(PROGS)
	$(QEMU) -M raspi3b -kernel $< -display none -S -s -serial null -serial stdio -initrd $(CPIO) -dtb $(DTB)

clean:
	rm $(PROGS) $(OBJ_FILES) $(BUILD_DIR)/$(ELF)