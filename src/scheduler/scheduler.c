#include "scheduler.h"

struct ProcessControlBlock new_pcb = {
};

struct ProcessControlBlock curent_pcb = {
};

uint32_t current_process_idx = 0;
uint32_t next_process_idx = 0;

void activate_timer_interrupt(void) {
    
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    // __asm__ volatile("sti");
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}

void timer_isr(struct InterruptFrame frame){

    struct Context c = {
        .cpu = frame.cpu,
        .eip = frame.int_stack.eip,
        .cs = frame.int_stack.cs,
        .eflags = frame.int_stack.eflags,
        .esp = frame.cpu.stack.esp,
        .ss = 0x20 | 0x3,
        .page_directory_virtual_addr = paging_get_current_page_directory_addr(),
    };

    scheduler_save_context_to_current_running_pcb(c);

    scheduler_switch_to_next_process();
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

    memcpy(&(*current_pcb).context, &ctx, sizeof(struct Context));
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void){
    // by default 0 selalu jalan karena itu tempatnya si shell.

    _process_list[current_process_idx].metadata.state = PROCESS_READY;
    
    current_process_idx = next_process_idx;

    // find process after current_process_idx
    uint32_t i = current_process_idx + 1;
    while(i != current_process_idx){
        
        if(process_manager_state._process_used[i] && _process_list[i].metadata.state == PROCESS_READY){
            next_process_idx = i;
            break;
        }
        i++;
        if(i >= PROCESS_COUNT_MAX){
            i = 0;
        }
    }

    if(i == current_process_idx){
        next_process_idx = current_process_idx;
    } else {
        next_process_idx = i;
    }

    // change state
    _process_list[current_process_idx].metadata.state = PROCESS_RUNNING;

    paging_use_page_directory(_process_list[current_process_idx].context.page_directory_virtual_addr);

    pic_ack(PIC1_OFFSET + IRQ_TIMER);
    process_context_switch(_process_list[current_process_idx].context);
    
    // if(current_process_idx == 0){
    //     _process_list[0].metadata.state = PROCESS_RUNNING;
        
    //     if (process_manager_state.active_process_count > 1){
    //         current_process_idx = 1;
    //         _process_list[1].metadata.state = PROCESS_READY;
    //     }
        
    //     paging_use_page_directory(_process_list[0].context.page_directory_virtual_addr);
    //     // process etc cleanup
    //     pic_ack(PIC1_OFFSET + IRQ_TIMER);
    //     process_context_switch(_process_list[0].context);
    // }
    // else if(process_manager_state.active_process_count > 1 && current_process_idx == 1){ // > 1 && current_process_idx == 1){
    //     _process_list[1].metadata.state = PROCESS_RUNNING;
    //     _process_list[0].metadata.state = PROCESS_READY;

    //     current_process_idx = 0;

    //     paging_use_page_directory(_process_list[1].context.page_directory_virtual_addr);
    //     // process etc cleanup
    //     pic_ack(PIC1_OFFSET + IRQ_TIMER);
    //     process_context_switch(_process_list[1].context);
    // }
}