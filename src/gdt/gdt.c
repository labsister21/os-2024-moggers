#include "gdt/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
static struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // NULL Descriptor
            .segment_low = 0,
            .segment_high = 0,

            .base_low = 0,
            .base_mid = 0,
            .base_high = 0,

            .type_bit = 0,
            .non_system = 0,
            .dpl_bit = 0,
            .present = 0,
            .available_bit = 0,
            .code_in_64 = 0,
            .db_flag = 0,
            .granularity = 0
        },
        {
            // Kernel Code Segment
            .segment_low = 0xFFFF,
            .segment_high = 0xF,

            .base_low = 0,
            .base_mid = 0,
            .base_high = 0,
            
            .type_bit = 0xA,
            .non_system = 1,
            .dpl_bit = 0,
            .present = 1,
            .available_bit = 0,
            .code_in_64 = 0,
            .db_flag = 1,
            .granularity = 1
        },
        {
            // Kernel Data Segment
            .segment_low = 0xFFFF,
            .segment_high = 0xF,

            .base_low = 0,
            .base_mid = 0,
            .base_high = 0,
            
            .type_bit = 0x2,
            .non_system = 1,
            .dpl_bit = 0,
            .present = 1,
            .available_bit = 0,
            .code_in_64 = 0,
            .db_flag = 1,
            .granularity = 1
        },
        {/* TODO: User   Code Descriptor */
            .segment_low = 0xFFFF,
            .segment_high = 0xF,

            .base_low = 0,
            .base_mid = 0,
            .base_high = 0,
            
            .type_bit = 0xA,
            .non_system = 1,
            .dpl_bit = 0x3,
            .present = 1,
            .available_bit = 0,
            .code_in_64 = 0,
            .db_flag = 1,
            .granularity = 1
        },
        {/* TODO: User   Data Descriptor */
            .segment_low = 0xFFFF,
            .segment_high = 0xF,

            .base_low = 0,
            .base_mid = 0,
            .base_high = 0,
            
            .type_bit = 0x2,
            .non_system = 1,
            .dpl_bit = 0x3,
            .present = 1,
            .available_bit = 0,
            .code_in_64 = 0,
            .db_flag = 1,
            .granularity = 1
        },
        {
            .segment_low       = sizeof(struct TSSEntry),
            .segment_high      = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,

            .non_system        = 0,    // S bit - non system
            .type_bit          = 0x9,
            .dpl_bit            = 0,    // DPL - priviledge
            .present            = 1,    // P bit - valid bit
            .db_flag            = 1,    // D/B bit - opr_32_bit
            .code_in_64         = 0,    // L bit - long mode
            .granularity        = 0,    // G bit
        },
        {0}
    }
};



/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
    .size = sizeof(global_descriptor_table) - 1,
    .address = &global_descriptor_table
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}