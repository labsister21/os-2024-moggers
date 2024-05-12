#include "process.h"
#include "paging/paging.h"
#include "std/string.h"
#include "gdt/gdt.h"

static struct ProcessManagerState process_manager_state = {
    .active_process_count = 0,
    ._process_used = {false}
};

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void){
    for(uint32_t i=0; i<PROCESS_COUNT_MAX; i++){
        if(process_manager_state._process_used[i] &&
        _process_list[i].metadata.state == RUNNING){
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

    new_pcb->metadata.pid = process_generate_new_pid();

exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid) {
    for(uint32_t i=0; i<PROCESS_COUNT_MAX; i++){
        if(_process_list[i].metadata.pid == pid){
            // destroy process
            process_manager_state.active_process_count--;
            process_manager_state._process_used[i] = false;


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