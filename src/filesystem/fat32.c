#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "std/string.h"
#include "fat32.h"

struct FAT32DriverState driver_state;

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', 'E', 'D', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};



uint32_t cluster_to_lba(uint32_t cluster){
    return cluster * CLUSTER_BLOCK_COUNT;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    struct FAT32DirectoryEntry dir_entry = {
        .attribute = ATTR_SUBDIRECTORY,
        .user_attribute = UATTR_NOT_EMPTY,
        .cluster_high = parent_dir_cluster >> 16,
        .cluster_low = parent_dir_cluster,
        .filesize = 0
    };
    memcpy(dir_entry.name, name, sizeof(dir_entry.name));

    struct FAT32DirectoryEntry parent_entry = {
        .name = {'.', '.', '\0', 0x0},
        .attribute = ATTR_SUBDIRECTORY,
        .user_attribute = UATTR_NOT_EMPTY,
        .cluster_high = parent_dir_cluster >> 16,
        .cluster_low = parent_dir_cluster,
        .filesize = 0
    };
    
    dir_table->table[0] = dir_entry;
    dir_table->table[1] = parent_entry;
}

bool is_empty_storage(void){
    struct ClusterBuffer boot_sector;
    read_clusters(boot_sector.buf, BOOT_SECTOR, 1);
    return memcmp(boot_sector.buf, fs_signature, BLOCK_SIZE);
}

void create_fat32(void){
    // write block signature
    write_blocks(fs_signature, BOOT_SECTOR, 1);
    
    // initialize fat allocation table
    struct FAT32FileAllocationTable fat_table = {
        .cluster_map = {
            CLUSTER_0_VALUE,
            CLUSTER_1_VALUE,
            FAT32_FAT_END_OF_FILE,
            0x00
        }
    };
    write_clusters(fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    // initialize root directory
    struct FAT32DirectoryTable root_dir;
    init_directory_table(&root_dir, "root", ROOT_CLUSTER_NUMBER);
    write_clusters(root_dir.table, ROOT_CLUSTER_NUMBER, 1);
}

void initialize_filesystem_fat32(void){
    if(is_empty_storage()){
        create_fat32();
    } else {
        read_clusters(driver_state.fat_table.cluster_map, 1, 1);
    }
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

