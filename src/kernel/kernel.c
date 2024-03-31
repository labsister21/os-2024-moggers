#include <stdint.h>
#include <stdbool.h>
#include "gdt/gdt.h"
#include "kernel/kernel-entrypoint.h"
#include "framebuffer/portio.h"
#include "framebuffer/framebuffer.h"
#include "interrupt/idt.h"
#include "interrupt/interrupt.h"
#include "keyboard/keyboard.h"
#include "filesystem/disk.h"
#include "filesystem/fat32.h"
#include "paging/paging.h"

void kernel_setup(void) {

    // FOR USER MODE TESTING
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_write(0, 0, ' ', 0xF, 0);
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Allocate first 4 MiB virtual memory
    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    read(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);
    while (true);
}


/*
void kernel_setup(void) {
    
    uint32_t a;
    uint32_t volatile b = 0x0000BABE;
    __asm__("mov $0xCAFE0000, %0" : "=r"(a));
    while (TRUE) b += 1;
   
    // load_gdt(&_gdt_gdtr);
    // while (true);
    
    // framebuffer_clear();
    // framebuffer_write(3, 8,  'H', 0, 0xF);
    // framebuffer_write(3, 9,  'a', 0, 0xF);
    // framebuffer_write(3, 10, 'i', 0, 0xF);
    // framebuffer_write(3, 11, '!', 0, 0xF);
    // framebuffer_set_cursor(3, 10);
    // while (true);
    

    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // __asm__("int $0x4");
    // framebuffer_clear();
    // framebuffer_set_cursor(0, 0);
    // framebuffer_write(0, 0, 'x', 0, 0xF);
    // while (true);
    
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    framebuffer_write(0, 0, '\0', 0xF, 0);
        
    keyboard_state_activate();
    // while(true);

    initialize_filesystem_fat32();
    struct FAT32DriverRequest request = {
        .buffer_size = 0,
        .name = {'e','d','\0', '\0', '\0', '\0', '\0', '\0'},
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    };

    uint8_t res = write(request);
    framebuffer_set_cursor(0, 0);
    framebuffer_write(0, 0, '0'+res, 0xF, 0);

    struct FAT32DriverRequest request2 = {
        .buffer_size = 0,
        .name = {'e','d', '2', '\0', '\0', '\0'},
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    };
    res = write(request2);
    framebuffer_set_cursor(0, 1);
    framebuffer_write(0, 1, '0'+res, 0xF, 0);

    int n = 4304;
    char input[n];
    for(int i=0; i<n; i++){
        if(i < 2048)
            input[i] = 'a';
        else if(i < 4096)
            input[i] = 'c';
        else
            input[i] = 'b';
    }
    input[n-1] = '\0';

    struct FAT32DriverRequest request3 = {
        .buf = input,
        .buffer_size = n,
        .name = {'e','d', '3', '\0', '\0', '\0'},
        .ext = {'t','x','t'},
        .parent_cluster_number = 0x3,
    };

    res = write(request3);
    framebuffer_set_cursor(0, 2);
    framebuffer_write(0, 2, '0'+res, 0xF, 0);


    res = delete(request2);
    framebuffer_set_cursor(0, 3);
    framebuffer_write(0, 3, '0'+res, 0xF, 0);

    int n2 = 3000;
    char input2[n2];
    for(int i=0; i<n2; i++){
        if (i < 2048)
            input2[i] = 'x';
        else
            input2[i] = 'y';
    }

    struct FAT32DriverRequest request4 = {
        .buf = input2,
        .buffer_size = n2,
        .name = {'e','d', '2', '\0', '\0', '\0'},
        .ext = {'p','h','p'},
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    };

    res = write(request4);
    framebuffer_set_cursor(0, 4);
    framebuffer_write(0, 4, '0'+res, 0xF, 0);

    res = delete(request3);
    framebuffer_set_cursor(0, 5);
    framebuffer_write(0, 5, '0'+res, 0xF, 0);


    // struct BlockBuffer b;
    // for (int i = 0; i < 512; i++) b.buf[i] = i % 16;
    // write_blocks(&b, 16, 1);
    while (true);
}
*/