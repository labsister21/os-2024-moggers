#include "idt.h"
#include "gdt/gdt.h"
/**
* interrupt_descriptor_table, predefined IDT.
* Initial IDTGate already set properly according to IDT Definition in 
Intel Manual & OSDev.
*/

struct InterruptDescriptorTable interrupt_descriptor_table;

/**
* _idt_idtr, predefined system IDTR.
* IDT pointed by this variable is already set to point interrupt_descriptor_table
above.
* From: https://wiki.osdev.org/Interrupt_Descriptor_Table, IDTR.size is IDT size minus
1.
*/
struct IDTR _idt_idtr = {
    // Use sizeof operator
    .size = sizeof(interrupt_descriptor_table) - 1,
    .address = &interrupt_descriptor_table
};

void set_interrupt_gate(uint8_t int_vector, void *handler_address, uint16_t gdt_seg_selector, uint8_t privilege){
    struct IDTGate* idt_int_gate = &interrupt_descriptor_table.table[int_vector];
    idt_int_gate->offset_low = ((uint32_t)handler_address & 0xFFFF);
    idt_int_gate->offset_high = ((uint32_t)handler_address >> 16);
    idt_int_gate->dpl_bit = privilege;
    idt_int_gate->segment = gdt_seg_selector;

    idt_int_gate->_r_bit_1 = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->_r_bit_2 = INTERRUPT_GATE_R_BIT_2;
    idt_int_gate->_r_bit_3 = INTERRUPT_GATE_R_BIT_3;
    idt_int_gate->gate_32 = 1;
    idt_int_gate->valid_bit = 1;
}

extern void* isr_stub_table[];

void initialize_idt(void){
    for (uint8_t i = 0; i < ISR_STUB_TABLE_LIMIT; i++) {
        if(i == 0x30)
            set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0x3);
        else
            set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0);
    }
    
    __asm__ volatile("lidt %0" : : "m"(_idt_idtr));
    __asm__ volatile("sti");
}