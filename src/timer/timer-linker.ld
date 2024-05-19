ENTRY(_start)

SECTIONS {
    . = 0x00000000; /* Assuming OS will load this program at virtual address */
    
    .text ALIGN(4):
    {
        crt0-timer.o(.text)  /* Put the entrypoint in the front of executable */
        *(.text)
    }

    .data ALIGN(4):
    {
        *(.data)
    }

    .bss ALIGN(4): {
        *(COMMON)
        *(.bss)
    }

    .rodata ALIGN(4):
    {
        *(.rodata*)
    }
    _linker_timer_end = .;
    ASSERT ((_linker_timer_end <= 1 * 1024 * 1024), "Error: _linker_timer_end linking result is more than 1 MiB")
}
