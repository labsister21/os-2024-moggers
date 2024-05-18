#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "paging.h"

// register CR3 nyimpen physical buat dipake.

// process protected memory. Ga bisa akses. Jadi pake paging. Misal 1 proses ada page table sendiri.
__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = { // intinya page table buat kernel
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
    }
};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {[0 ... PAGE_FRAME_MAX_COUNT-1] = false},
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT,
    // TODO: Fill in if needed ...
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}



/* --- Memory Management --- */
// TODO: Implement
bool paging_allocate_check(uint32_t amount) {
    // TODO: Check whether requested amount is available
    if(amount > page_manager_state.free_page_frame_count)
        return false;

    return true;
}


uint32_t paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */ 
    
    // using first fit algorithm - ketemu pertama x langsung dipake
    uint32_t free_physical_frame_index = 0;
    int i;
    for(i=0; i<PAGE_FRAME_MAX_COUNT; i++){
        if(!page_manager_state.page_frame_map[i]){
            page_manager_state.page_frame_map[i] = true;
            page_manager_state.free_page_frame_count--;
<<<<<<< HEAD
            free_physical_frame_index = i;
=======
>>>>>>> 30f8f85fa01c8424a332730b60299f537e9c414c
            break;
        }
    }

    // error handling no free space;
    if(i == PAGE_FRAME_MAX_COUNT)
        return -1;

    struct PageDirectoryEntryFlag user_flag = {
        .present_bit = true,
        .write_bit = true,
        .user_supervisor_bit = true,
        .pwt_bit = 0,
        .pcd_bit = 0,
        .accessed_bit = 0,
        .dirty_bit = 0,
        .use_pagesize_4_mb = true,
    };

    update_page_directory_entry(
        page_dir,
<<<<<<< HEAD
        (void *) (free_physical_frame_index * PAGE_FRAME_SIZE),
=======
        (void *) free_physical_frame_index,
>>>>>>> 30f8f85fa01c8424a332730b60299f537e9c414c
        virtual_addr,
        user_flag
    );

    return free_physical_frame_index;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /* 
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */

    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    uint32_t physical_address_index = 0;

    int i;
    for(i=0; i<PAGE_FRAME_MAX_COUNT; i++){
        if(page_dir->table[page_index].lower_address == (((uint32_t) i >> 22) & 0x3FF)){
            physical_address_index = i;
            page_manager_state.page_frame_map[i] = false;
            page_manager_state.free_page_frame_count++;

            memset(&(*page_dir).table[page_index], 0x0, sizeof(struct PageDirectoryEntry));
            break;
        }
    }

    // error handling not found;
    if(i == PAGE_FRAME_MAX_COUNT)
        return false;

    return true;
}




// ini array simpem pagedirectry === maximum ada 32 proses. ini static reuse.
__attribute__((aligned(0x1000))) static struct PageDirectory page_directory_list[PAGING_DIRECTORY_TABLE_MAX_COUNT] = {0};

/// TRACKER aja buat 32 proses itu.
static struct {
    bool page_directory_used[PAGING_DIRECTORY_TABLE_MAX_COUNT];
} page_directory_manager = {
    .page_directory_used = {false},
};

struct PageDirectory* paging_create_new_page_directory(void) {
    /*
     * TODO: Get & initialize empty page directory from page_directory_list
     * - Iterate page_directory_list[] & get unused page directory
     * - Mark selected page directory as used
     * - Create new page directory entry for kernel higher half with flag:
     *     > present bit    true
     *     > write bit      true
     *     > pagesize 4 mb  true
     *     > lower address  0
     * - Set page_directory.table[0x300] with kernel page directory entry
     * - Return the page directory address
     */ 
    for(uint32_t i=0; i<PAGING_DIRECTORY_TABLE_MAX_COUNT; i++){
        if(!page_directory_manager.page_directory_used[i]){

            page_directory_manager.page_directory_used[i] = true;

            struct PageDirectory* new_directory = &page_directory_list[i];

            struct PageDirectoryEntry new_entry = {
                .flag.present_bit       = 1,
                .flag.write_bit         = 1,
                .flag.use_pagesize_4_mb = 1,
                .lower_address          = 0,
            };

<<<<<<< HEAD
            new_directory->table[0x300] = new_entry;

            // memcpy(&new_directory->table[0x300], &new_entry, sizeof(struct PageDirectoryEntry));
=======
            memcpy(&new_directory->table[0x300], &new_entry, sizeof(struct PageDirectoryEntry));
>>>>>>> 30f8f85fa01c8424a332730b60299f537e9c414c

            return new_directory;
        }
    }
    return NULL;
}

bool paging_free_page_directory(struct PageDirectory *page_dir) {
    /**
     * TODO: Iterate & clear page directory values
     * - Iterate page_directory_list[] & check &page_directory_list[] == page_dir
     * - If matches, mark the page directory as unusued and clear all page directory entry
     * - Return true
     */
    for(uint32_t i=0; i<PAGING_DIRECTORY_TABLE_MAX_COUNT; i++){
        if(&page_directory_list[i] == page_dir){
            page_directory_manager.page_directory_used[i] = false;

            
            memset(page_dir, 0x0, sizeof(struct PageDirectory));
        }
    }
    return false;
}

struct PageDirectory* paging_get_current_page_directory_addr(void) {
    uint32_t current_page_directory_phys_addr;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_page_directory_phys_addr): /* <Empty> */);
    uint32_t virtual_addr_page_dir = current_page_directory_phys_addr + KERNEL_VIRTUAL_ADDRESS_BASE;
    return (struct PageDirectory*) virtual_addr_page_dir;
}

void paging_use_page_directory(struct PageDirectory *page_dir_virtual_addr) {
    uint32_t physical_addr_page_dir = (uint32_t) page_dir_virtual_addr;
    // Additional layer of check & mistake safety net
    if ((uint32_t) page_dir_virtual_addr > KERNEL_VIRTUAL_ADDRESS_BASE)
        physical_addr_page_dir -= KERNEL_VIRTUAL_ADDRESS_BASE;
    __asm__  volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(physical_addr_page_dir): "memory");
}