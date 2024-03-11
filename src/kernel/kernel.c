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


void kernel_setup(void) {
    /*
    uint32_t a;
    uint32_t volatile b = 0x0000BABE;
    __asm__("mov $0xCAFE0000, %0" : "=r"(a));
    while (TRUE) b += 1;
    */
   
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
        .name = {'e','d','\0'},
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    };

    uint8_t res = write(request);
    framebuffer_set_cursor(0, 0);
    framebuffer_write(0, 0, '0'+res, 0xF, 0);

    struct FAT32DriverRequest request2 = {
        .buffer_size = 0,
        .name = {'e','d', '2', '\0'},
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    };
    res = write(request2);
    framebuffer_set_cursor(0, 1);
    framebuffer_write(0, 1, '0'+res, 0xF, 0);

    char input[1000];
    for(int i=0; i<100; i++){
        input[i] = 'a';
    }
    struct FAT32DriverRequest request3 = {
        .buf = input,
        .buffer_size = 1000,
        .name = {'e','d', '3', '\0'},
        .ext = {'t','x','t'},
        .parent_cluster_number = 0x3,
    };

    res = write(request3);
    framebuffer_set_cursor(0, 2);
    framebuffer_write(0, 2, '0'+res, 0xF, 0);


    res = delete(request2);
    framebuffer_set_cursor(0, 2);
    framebuffer_write(0, 2, '0'+res, 0xF, 0);
    // struct BlockBuffer b;
    // for (int i = 0; i < 512; i++) b.buf[i] = i % 16;
    // write_blocks(&b, 16, 1);
    while (true);
}