#include <stdint.h>
#include <stdbool.h>
#include "gdt/gdt.h"
#include "kernel/kernel-entrypoint.h"


void kernel_setup(void) {
    /*
    uint32_t a;
    uint32_t volatile b = 0x0000BABE;
    __asm__("mov $0xCAFE0000, %0" : "=r"(a));
    while (TRUE) b += 1;
    */

    load_gdt(&_gdt_gdtr);
    while (true);
}