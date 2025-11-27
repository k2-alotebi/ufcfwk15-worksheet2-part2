# Makefile for Tiny OS - Worksheet 2 Part 2

# Compiler and linker settings
CC = gcc
CFLAGS = -I. -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T ./source/link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

# Object files
OBJECTS = source/loader.o \
          source/kmain.o \
          drivers/io.o \
          drivers/frame_buffer.o \
          drivers/hardware_interrupt_enabler.o \
          drivers/interrupt_asm.o \
          drivers/interrupt_handlers.o \
          drivers/interrupts.o \
          drivers/keyboard.o \
          drivers/pic.o \
          drivers/input_buffer.o \
          drivers/terminal.o

.PHONY: all clean run run-curses run-simple stop kill-port viewlog

all: os.iso

# Compile assembly files
source/loader.o: source/loader.asm
	$(AS) $(ASFLAGS) source/loader.asm -o source/loader.o

drivers/io.o: drivers/io.s
	$(AS) $(ASFLAGS) drivers/io.s -o drivers/io.o

drivers/hardware_interrupt_enabler.o: drivers/hardware_interrupt_enabler.s
	$(AS) $(ASFLAGS) drivers/hardware_interrupt_enabler.s -o drivers/hardware_interrupt_enabler.o

drivers/interrupt_asm.o: drivers/interrupt_asm.s
	$(AS) $(ASFLAGS) drivers/interrupt_asm.s -o drivers/interrupt_asm.o

drivers/interrupt_handlers.o: drivers/interrupt_handlers.s
	$(AS) $(ASFLAGS) drivers/interrupt_handlers.s -o drivers/interrupt_handlers.o

# Compile C files
source/kmain.o: source/kmain.c
	$(CC) $(CFLAGS) source/kmain.c -o source/kmain.o

drivers/frame_buffer.o: drivers/frame_buffer.c
	$(CC) $(CFLAGS) drivers/frame_buffer.c -o drivers/frame_buffer.o

drivers/interrupts.o: drivers/interrupts.c
	$(CC) $(CFLAGS) drivers/interrupts.c -o drivers/interrupts.o

drivers/keyboard.o: drivers/keyboard.c
	$(CC) $(CFLAGS) drivers/keyboard.c -o drivers/keyboard.o

drivers/pic.o: drivers/pic.c
	$(CC) $(CFLAGS) drivers/pic.c -o drivers/pic.o

drivers/input_buffer.o: drivers/input_buffer.c
	$(CC) $(CFLAGS) drivers/input_buffer.c -o drivers/input_buffer.o

drivers/terminal.o: drivers/terminal.c
	$(CC) $(CFLAGS) drivers/terminal.c -o drivers/terminal.o

# Link kernel
kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# Build ISO image
os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R \
		-b boot/grub/stage2_eltorito \
		-no-emul-boot \
		-boot-load-size 4 \
		-A os \
		-input-charset utf8 \
		-quiet \
		-boot-info-table \
		-o os.iso \
		iso

# Run the OS in QEMU - nographic mode
run: os.iso
	qemu-system-i386 -nographic -boot d -cdrom os.iso -m 32 -d cpu -D logQ.txt

# Alternative run command if -display curses doesn't work (use VNC instead)
run-vnc: os.iso
	qemu-system-i386 \
		-vnc :0 \
		-monitor telnet::45454,server,nowait \
		-boot d \
		-cdrom os.iso \
		-m 32 \
		-d cpu \
		-no-reboot \
		-no-shutdown \
		-D logQ.txt

# Run the OS in QEMU - curses mode (required for framebuffer)
# In another terminal, connect with: telnet localhost 45454
# Then type: quit
# IMPORTANT: Make sure QEMU window has focus for keyboard input!
run-curses: os.iso kill-port
	@echo "Starting QEMU... Wait 3 seconds, then in another terminal run: telnet localhost 45454"
	@echo "To quit QEMU from telnet, type: quit"
	@echo "IMPORTANT: Click in QEMU window to give it keyboard focus!"
	qemu-system-i386 \
		-display curses \
		-k en-us \
		-monitor telnet::45454,server,nowait \
		-serial mon:stdio \
		-boot d \
		-cdrom os.iso \
		-m 32 \
		-d cpu \
		-no-reboot \
		-no-shutdown \
		-D logQ.txt

# Run in simple curses mode (easier to quit - press ESC+2 then type 'quit')
run-simple: os.iso kill-port
	qemu-system-i386 \
		-display curses \
		-boot d \
		-cdrom os.iso \
		-m 32

stop:
	@-pkill -9 -f qemu 2>/dev/null || true
	@-fuser -k 45454/tcp 2>/dev/null || true
	@-lsof -ti:45454 2>/dev/null | xargs -r kill -9 2>/dev/null || true
	@echo "QEMU stopped"

kill-port:
	@-pkill -9 -f qemu 2>/dev/null || true
	@-fuser -k 45454/tcp 2>/dev/null || true
	@-lsof -ti:45454 2>/dev/null | xargs -r kill -9 2>/dev/null || true
	@-ss -K dst :45454 2>/dev/null || true
	@sleep 2
	@echo "Port 45454 cleared"

viewlog:
	@cat logQ.txt

# Clean build files
clean:
	rm -f source/*.o drivers/*.o kernel.elf os.iso logQ.txt
	rm -f iso/boot/kernel.elf
