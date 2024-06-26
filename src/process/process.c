#include "process.h"
#include "paging/paging.h"
#include "std/string.h"
#include "gdt/gdt.h"

struct ProcessManagerState process_manager_state = {
    .active_process_count = 0,
    ._process_used = {false}
};

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void){
    for(uint32_t i=0; i<PROCESS_COUNT_MAX; i++){
        if(process_manager_state._process_used[i] &&
        _process_list[i].metadata.state == PROCESS_RUNNING){
            return &_process_list[i];
        }
    }
    return NULL;
}

struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX];

int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = PROCESS_CREATE_SUCCESS; 
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t) request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE) {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB 
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);

    memcpy(&new_pcb->metadata.name, request.name, 8);
    new_pcb->metadata.pid = process_generate_new_pid();
    new_pcb->metadata.state = PROCESS_READY;

    struct PageDirectory* current_pd = paging_get_current_page_directory_addr();
    // create new Virtual Address
    struct PageDirectory* new_pd = paging_create_new_page_directory();

    new_pcb->memory.virtual_addr_used[0] = paging_allocate_user_page_frame(new_pd, request.buf) + KERNEL_VIRTUAL_ADDRESS_BASE; 
    new_pcb->memory.virtual_addr_used[1] = paging_allocate_user_page_frame(new_pd, 0xBFFFFFFC) + KERNEL_VIRTUAL_ADDRESS_BASE;
    new_pcb->memory.page_frame_used_count = 2;
    
    process_manager_state._process_used[p_index] = true;
    process_manager_state.active_process_count++;

    // ganti ke virtual address baru
    paging_use_page_directory(new_pd);

    // Membaca dan melakukan load executable dari file system ke memory baru
    uint32_t res_code = read(request);

    if(!!res_code){
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }

    paging_use_page_directory(current_pd);

    // Menyiapkan state & context awal untuk program
    // Segment register seharusnya menggunakan nilai Segment Selector yang menunjuk ke GDT user data segment descriptor dan memiliki Privilege Level 3

    // segment register ds, es, fs, gs
    // GDT use data segment descriptor priv 3
    // 0000000000100 0 11 = 0x20 | 0x3 = 35
    
    // initialize context bukannya sudah ada di kernel_execute_user_program ?
    new_pcb->context.cpu.segment.ds = 0x20 | 0x3;
    new_pcb->context.cpu.segment.es = 0x20 | 0x3;
    new_pcb->context.cpu.segment.fs = 0x20 | 0x3;
    new_pcb->context.cpu.segment.gs = 0x20 | 0x3;
    new_pcb->context.eip = (uint32_t) request.buf;

    new_pcb->context.cpu.stack.ebp = 0xBFFFFFFC;
    new_pcb->context.cpu.stack.esp = 0xBFFFFFFC;

    new_pcb->context.page_directory_virtual_addr = new_pd;

    new_pcb->context.cs = 0x18 | 0x3;
    new_pcb->context.esp = 0xBFFFFFFC;
    new_pcb->context.ss = 0x20 | 0x3;
    
    new_pcb->context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;

    // setup metadata
    new_pcb->metadata.state = PROCESS_READY;

exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid) {
    for(uint32_t i=0; i<PROCESS_COUNT_MAX; i++){
        if(_process_list[i].metadata.pid == pid){
            // destroy process
            process_manager_state.active_process_count--;
            process_manager_state._process_used[i] = false;

            // release page frame
            for(uint8_t j=0; j< PROCESS_PAGE_FRAME_COUNT_MAX; j++){
                // if(_process_list[i].memory.virtual_addr_used[j])
                paging_free_user_page_frame(_process_list[pid].context.page_directory_virtual_addr, _process_list[pid].memory.virtual_addr_used[j]);
            }

            // free page directory
            paging_free_page_directory(_process_list[pid].context.page_directory_virtual_addr);

            // clean the page_directory
            memset(&_process_list[i], 0x0, sizeof(struct ProcessControlBlock));

            return true;
        }
    }
    return false;
}

uint32_t process_list_get_inactive_index(){
    for(uint32_t i=0; i<PROCESS_COUNT_MAX; i++){
        if(!process_manager_state._process_used[i]){
            return i;
        }
    }
    return -1;
}

uint32_t process_generate_new_pid(){
    uint32_t pid = process_list_get_inactive_index();
    return pid;
}

uint32_t ceil_div(uint32_t a, uint32_t b){
    uint32_t c = !!(a % b);
    return (a / b) + c;
}