# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Disk
DISK_NAME	  = storage

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = os2024

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386



run: all
	@qemu-system-i386 -s -S -drive file=bin/storage.bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
all: build
build: iso
clean:
	@cd bin && rm -rf *.o *.iso $(OUTPUT_FOLDER)/kernel

kernel:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/interrupt/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel/kernel-entrypoint.s -o $(OUTPUT_FOLDER)/kernel-entrypoint.o
# TODO: Compile C file with CFLAGS
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/kernel/kernel.c -o $(OUTPUT_FOLDER)/kernel.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/std/string.c -o $(OUTPUT_FOLDER)/string.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/gdt/gdt.c -o $(OUTPUT_FOLDER)/gdt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/framebuffer/portio.c -o $(OUTPUT_FOLDER)/portio.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/framebuffer/framebuffer.c -o $(OUTPUT_FOLDER)/framebuffer.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/interrupt.c -o $(OUTPUT_FOLDER)/interrupt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/syscall_command.c -o $(OUTPUT_FOLDER)/syscall_command.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/idt.c -o $(OUTPUT_FOLDER)/idt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/keyboard/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/disk.c -o $(OUTPUT_FOLDER)/disk.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/fat32.c -o $(OUTPUT_FOLDER)/fat32.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/paging/paging.c -o $(OUTPUT_FOLDER)/paging.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/process/process.c -o $(OUTPUT_FOLDER)/process.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/scheduler/scheduler.c -o $(OUTPUT_FOLDER)/scheduler.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/cmos/cmos.c -o $(OUTPUT_FOLDER)/cmos.o
	@$(LIN) $(LFLAGS) bin/*.o -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f *.o

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
# TODO: Create ISO image
	@cd $(OUTPUT_FOLDER) && genisoimage -R -b boot/grub/grub1 -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os2024.iso iso
	@rm -r $(OUTPUT_FOLDER)/iso/

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g \
		$(SOURCE_FOLDER)/std/string.c $(SOURCE_FOLDER)/filesystem/fat32.c \
		$(SOURCE_FOLDER)/inserter/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter

user-shell:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/user_mode/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/user-shell.c -o user-shell.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/cat.c -o cat.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/ls.c -o ls.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/mkdir.c -o mkdir.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/cd.c -o cd.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/clear.c -o clear.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/help.c -o help.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/rm.c -o rm.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/mv.c -o mv.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/find.c -o find.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/exec.c -o exec.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/apple.c -o apple.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user_mode/command_list/kill.c -o kill.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/std/string.c -o string.o
	@$(LIN) -T $(SOURCE_FOLDER)/user_mode/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o user-shell.o cat.o ls.o mkdir.o cd.o clear.o help.o rm.o mv.o find.o exec.o apple.o kill.o string.o -o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/user_mode/user-linker.ld -melf_i386 --oformat=elf32-i386\
		crt0.o user-shell.o cat.o ls.o mkdir.o cd.o clear.o help.o rm.o mv.o find.o exec.o apple.o kill.o string.o -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...
	@size --target=binary bin/shell
	@rm -f *.o

user-timer:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/timer/crt0-timer.s -o crt0-timer.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/timer/timer.c -o timer.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/std/string.c -o string.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/cmos/cmos.c -o cmos.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/framebuffer/portio.c -o portio.o
	
	@$(LIN) -T $(SOURCE_FOLDER)/timer/timer-linker.ld -melf_i386 --oformat=binary \
		crt0-timer.o timer.o cmos.o portio.o string.o -o $(OUTPUT_FOLDER)/timer
	@echo Linking object shell object files and generate flat binary...

#	@$(LIN) -T $(SOURCE_FOLDER)/timer/timer-linker.ld -melf_i386 --oformat=elf32-i386\
		crt0-timer.o timer.o cmos.o portio.o string.o -o $(OUTPUT_FOLDER)/timer_elf
	@echo Linking object timer object files and generate ELF32 for debugging...
	@size --target=binary bin/timer
	@rm -f *.o

user-bounce:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/bounce/crt0-bounce.s -o crt0-bounce.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/bounce/bounce.c -o bounce.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/std/string.c -o string.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/cmos/cmos.c -o cmos.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/framebuffer/portio.c -o portio.o
	
	@$(LIN) -T $(SOURCE_FOLDER)/bounce/bounce-linker.ld -melf_i386 --oformat=binary \
		crt0-bounce.o bounce.o cmos.o portio.o string.o -o $(OUTPUT_FOLDER)/bounce
	@echo Linking object shell object files and generate flat binary...

#	@$(LIN) -T $(SOURCE_FOLDER)/timer/timer-linker.ld -melf_i386 --oformat=elf32-i386\
		crt0-timer.o timer.o cmos.o portio.o string.o -o $(OUTPUT_FOLDER)/timer_elf
	@echo Linking object bounce object files and generate ELF32 for debugging...
	@size --target=binary bin/bounce
	@rm -f *.o


insert-timer: inserter user-timer
	@cd $(OUTPUT_FOLDER); ./inserter timer 2 $(DISK_NAME).bin

insert-bounce: inserter user-bounce
	@cd $(OUTPUT_FOLDER); ./inserter bounce 2 $(DISK_NAME).bin

insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin
	@cd $(OUTPUT_FOLDER); ./inserter edbert 2 $(DISK_NAME).bin
	@cd $(OUTPUT_FOLDER); ./inserter edbert2 2 $(DISK_NAME).bin
	@cd $(OUTPUT_FOLDER); ./inserter os-dev 2 $(DISK_NAME).bin
