#include "scheduler.h"

struct ProcessControlBlock new_pcb = {
    .context.cpu.general.eax = 1,
    .context.cpu.general.ebx = 2,
    .context.cpu.general.ecx = 3,
    .context.cpu.general.edx = 4,
    .context.eflags = 5,
    .context.eip = 6,
    .context.page_directory_virtual_addr = 7
};

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}

void timer_isr(void){
    scheduler_switch_to_next_process();
    pic_ack(PIC1_OFFSET + IRQ_TIMER);
}

void idle_main(void* arg) {
    while (true)
        asm("hlt");
}

void scheduler_init(void){
    activate_timer_interrupt();
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx){
    struct ProcessControlBlock *current_pcb = process_get_current_running_pcb_pointer();

    memset(&(*current_pcb).context, (int) &ctx, sizeof(struct Context));
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void){
    process_context_switch(new_pcb.context);
}