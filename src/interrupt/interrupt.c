#include "interrupt.h"
#include "keyboard/keyboard.h"
#include "scheduler/scheduler.h"
#include "filesystem/fat32.h"
#include "syscall_command.h"

// initialize _interrupt_tss_entry 
struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}


void syscall(struct InterruptFrame frame) {

    /* 0 - read() - function to read (file) */
    if (frame.cpu.general.eax == 0) {
        *((int8_t*) frame.cpu.general.ecx) = read(
            *(struct FAT32DriverRequest*) frame.cpu.general.ebx
        );
    }
    /* 1 - read_directory() - function to read directory */
    else if(frame.cpu.general.eax == 1) {
        *((int8_t*) frame.cpu.general.ecx) = read_directory(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
    }
    /* 2 - write() - function to write to file or directory (directory is also a file) */
    else if(frame.cpu.general.eax == 2) {
        *((int8_t*) frame.cpu.general.ecx) = write(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
    }
    /* 3 - delete() - function to delete file or directory (directory is also a file) */
    else if(frame.cpu.general.eax == 3) {
        *((int8_t*) frame.cpu.general.ecx) = delete(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
    }
    /* 4 - getchar() - function to get char */
    else if (frame.cpu.general.eax == 4) {
        // keyboard_state_activate();
        // TODO: getchar()
        get_keyboard_buffer((char*) frame.cpu.general.ebx);

    }
    /* 5 - putchar() - function to write char */
    else if (frame.cpu.general.eax == 5) {
        putchar(
            (char*) frame.cpu.general.ebx,
            frame.cpu.general.ecx,
            (CP*) frame.cpu.general.edx
        );

    }
    /* 6 - puts() - function to write to screen */
    else if (frame.cpu.general.eax == 6) {
        puts(
            (char*) frame.cpu.general.ebx, 
            (putsExtraAttribute*) frame.cpu.general.ecx, 
            (CP*) frame.cpu.general.edx
        ); // Assuming puts() exist in kernel

    }
    /* 7 - Activate Keyboard Input */
    else if(frame.cpu.general.eax == 7) {
        keyboard_state_activate();
    
    }
    /* 8 - move_text_cursor() - function to move text cursor to (x, y) coordinates */
    else if(frame.cpu.general.eax == 8) {
        move_text_cursor(
            frame.cpu.general.ebx,
            frame.cpu.general.ecx,
            frame.cpu.general.edx
        );
    }
    /* 9 - clear_screen() - function to clear the screen */
    else if(frame.cpu.general.eax == 9) {
        clear_screen();
    }
    /* 10 - get_cursor_position - debug aja dari edbert */
    else if(frame.cpu.general.eax == 10) {
        *((uint16_t*) frame.cpu.general.ebx) = get_cursor_position();
    }
}


void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PAGE_FAULT:
            __asm__("hlt");
            break;
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case PIC1_OFFSET + IRQ_TIMER:
            timer_isr();
            break;
        case 0x30:
            syscall(frame);
            break;
    }
}
