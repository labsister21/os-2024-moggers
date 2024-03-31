#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../std/string.h"
#include "fat32.h"

struct FAT32DriverState driver_state;

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '3', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

/*
+---------------------------------------------------+
|             FAT32 DRIVER INTERFACES               |
+---------------------------------------------------+
*/

uint32_t cluster_to_lba(uint32_t cluster){
    return cluster * CLUSTER_BLOCK_COUNT;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    struct FAT32DirectoryEntry dir_entry = {
        .ext = { 0x0 },
        .attribute = ATTR_SUBDIRECTORY,
        .user_attribute = UATTR_NOT_EMPTY,

        .undelete = 0x0,
        .create_time = 0x0,
        .create_date = 0x0,
        .access_date = 0x0,

        .modified_time = 0x0,
        .modified_date = 0x0,

        .cluster_high = parent_dir_cluster >> 16,
        .cluster_low = parent_dir_cluster,

        .filesize = 0
    };
    memcpy(dir_entry.name, name, sizeof(dir_entry.name));

    struct FAT32DirectoryEntry parent_entry = {
        .name = {'.', '.', '\0', '\0', '\0', '\0', '\0', '\0'},
        .ext = {0x00},
        .attribute = ATTR_SUBDIRECTORY,
        .user_attribute = UATTR_NOT_EMPTY,

        .undelete = 0x0,
        .create_time = 0x0,
        .create_date = 0x0,
        .access_date = 0x0,

        .modified_time = 0x0,
        .modified_date = 0x0,

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
    memset(root_dir.table, 0x00, sizeof(root_dir.table));
    
    char dir_name[] = {'r', 'o', 'o', 't', '\0', '\0', '\0', '\0'};
    init_directory_table(&root_dir, dir_name, ROOT_CLUSTER_NUMBER);
    write_clusters(root_dir.table, ROOT_CLUSTER_NUMBER, 1);

    // init driver
    driver_state.fat_table = fat_table;
    driver_state.dir_table_buf = root_dir;
}

void initialize_filesystem_fat32(void){
    if(is_empty_storage()){
        create_fat32();
    } else {
        read_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    }
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}


/*
+---------------------------------------------------+
|                   CRUD OPERATION                  |
+---------------------------------------------------+
*/

int8_t read_directory(struct FAT32DriverRequest request){
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    // check if directory empty or not
    if(driver_state.dir_table_buf.table->user_attribute != UATTR_NOT_EMPTY){ // directory is empty
        return -1;
    }

    // search for directory with the same name
    uint32_t i;
    for(i=0; i < (sizeof(driver_state.dir_table_buf)/sizeof(struct FAT32DirectoryEntry)); i++){
        // check dir name
        if(memcmp(driver_state.dir_table_buf.table[i].name, request.name, 8) == 0){
            // check if its a file or a folder
            if(driver_state.dir_table_buf.table[i].user_attribute != ATTR_SUBDIRECTORY){
                // return not a folder
                return 1;
            }
            
            // all checked passed. Read folder
            uint16_t cluster_high = driver_state.dir_table_buf.table[i].cluster_high;
            uint16_t cluster_low = driver_state.dir_table_buf.table[i].cluster_low;
            uint32_t cluster_num = (cluster_high << 16) + cluster_low;

            read_clusters(request.buf, cluster_num, 1);
            
            // succeed!
            return 0;
        }
    }

    // folder not found
    return 2;
}

int8_t read(struct FAT32DriverRequest request){
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    // check if directory empty or not
    if(driver_state.dir_table_buf.table->user_attribute != UATTR_NOT_EMPTY){ // directory is empty
        return -1;
    }
    
    // read FAT
    read_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    // search file inside directory
    uint32_t i;
    for(i=0; i < (sizeof(driver_state.dir_table_buf)/sizeof(struct FAT32DirectoryEntry)); i++){
        if(memcmp(driver_state.dir_table_buf.table[i].name, request.name, 8) == 0 && // check filename
            memcmp(driver_state.dir_table_buf.table[i].ext, request.ext, 3) == 0){ // check extention
                // check if its a file or a folder
                if(driver_state.dir_table_buf.table[i].user_attribute == ATTR_SUBDIRECTORY){
                    // return not a file
                    return 1;
                }
                // check if request.buffer_size big enough
                if(request.buffer_size < driver_state.dir_table_buf.table[i].filesize){
                    // buffer not enough.
                    // bacanya: Dari DirectoryEntry.filesize dimasukin ke request.buffer_size
                    return -1;
                }

                // all checked passed. Read data
                uint16_t cluster_high = driver_state.dir_table_buf.table[i].cluster_high;
                uint16_t cluster_low = driver_state.dir_table_buf.table[i].cluster_low;
                uint32_t cluster_num = (cluster_high << 16) + cluster_low;

                uint16_t counter = 0;
                do {
                    read_clusters(request.buf + CLUSTER_SIZE * counter, cluster_num, 1);
                    counter++;
                    cluster_num = driver_state.fat_table.cluster_map[cluster_num];
                } while(cluster_num != FAT32_FAT_END_OF_FILE);

                // operation succeed !!!
                return 0;
            }
    }
    
    // file not found
    return 2;
}

int8_t write(struct FAT32DriverRequest request){
    // TODO: tambah handling input biar data integrity kejaga.


    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    // check if parent directory is not a directory
    if( !(driver_state.dir_table_buf.table->user_attribute == UATTR_NOT_EMPTY &&
          driver_state.dir_table_buf.table->attribute == ATTR_SUBDIRECTORY)){
        // not a directory
        return 2;
    }

    // check if full or not
    int i;
    bool isFull = true;
    int entry_row;
    // start from 2. entry 0, 1 already used.
    for(i=2; i<64; i++){
        if(driver_state.dir_table_buf.table[i].user_attribute != UATTR_NOT_EMPTY){
            isFull = false;
            entry_row = i;
            break;
        }
    }
    // is dir full : uncategorized
    if(isFull) return -1;

    bool isExist = false;
    // check if file name & ext already exist
    for(i=2; i<64; i++){
        if(memcmp(driver_state.dir_table_buf.table[i].name, request.name, 8) == 0 && // check name
           memcmp(driver_state.dir_table_buf.table[i].ext, request.ext, 3) == 0){ // check ext
            isExist = true;
            break;
           }
    }
    // is file already exist
    if(isExist) return 1;


    // handle if create folder
    if(request.buffer_size == 0){
        uint32_t cluster_number = 0x0;
        // find empty cluster in fat table
        for(i=0; i<CLUSTER_SIZE; i++){
            if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY){
                cluster_number = i;
                break;
            }
        }

        // if there is no available cluster return -1 error. 
        if(cluster_number == 0) return -1;

        // update fat table
        driver_state.fat_table.cluster_map[cluster_number] = FAT32_FAT_END_OF_FILE;
        write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

        // update parent directory
        struct FAT32DirectoryTable new_dir;
        memset(new_dir.table, 0x00, sizeof(new_dir.table));
        init_directory_table(&new_dir, request.name,  request.parent_cluster_number);
        new_dir.table[0].cluster_high = cluster_number >> 16;
        new_dir.table[0].cluster_low = cluster_number;

        // add to parent table allocation
        driver_state.dir_table_buf.table[entry_row] = new_dir.table[0];
        write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

        // create new table section
        write_clusters(new_dir.table, cluster_number, 1);

        // succeed
        return 0;
    }

    // handle create file
    int cluster_needed = request.buffer_size / CLUSTER_SIZE;
    int mod = request.buffer_size % CLUSTER_SIZE;
    if(mod != 0) cluster_needed++;

    uint32_t temp_cluster = 0;
    // get first cluster in fat table
    for(i=0; i<CLUSTER_SIZE; i++){
        if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY){
            // directly write
            temp_cluster = i;
            cluster_needed--;
            break;
        }
    }
    uint32_t start_cluster = temp_cluster;
    int file_part = 0;

    // create cluster buffer
    struct ClusterBuffer cluster_buffer;
    memset(cluster_buffer.buf, 0x0, CLUSTER_SIZE);
    memcpy(cluster_buffer.buf, (uint8_t*) request.buf + CLUSTER_SIZE*file_part, CLUSTER_SIZE);
    
    // write to cluster
    write_clusters((uint8_t*) request.buf + CLUSTER_SIZE*file_part, start_cluster, 1);
    driver_state.fat_table.cluster_map[start_cluster] = FAT32_FAT_END_OF_FILE;
    file_part++;

    // fill all cluster
    for(i=start_cluster+1; i<CLUSTER_SIZE && cluster_needed > 0; i++){
        if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY){
            // directly write
            driver_state.fat_table.cluster_map[temp_cluster] = i;
            temp_cluster = i;
            
            memset(cluster_buffer.buf, 0x0, sizeof(cluster_buffer.buf));

            int cluster_buffer_size = CLUSTER_SIZE;
            if(cluster_needed == 1) cluster_buffer_size = mod;
            memcpy(cluster_buffer.buf, (uint8_t*) request.buf + CLUSTER_SIZE*file_part, cluster_buffer_size);
            
            write_clusters(cluster_buffer.buf, temp_cluster, 1);
            driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE;
            
            file_part++;
            cluster_needed--;
        }
    }

    // write update FAT cluster
    write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    // update directory table
    struct FAT32DirectoryEntry new_file = {
        .name = {request.name[0], request.name[1], request.name[2], request.name[3], request.name[4], request.name[5], request.name[6], request.name[7]},
        .ext = {request.ext[0], request.ext[1], request.ext[2]},
        .attribute = 0x0,
        .user_attribute = UATTR_NOT_EMPTY,

        .undelete = 0x0,
        .create_time = 0x0,
        .create_date = 0x0,
        .access_date = 0x0,

        .modified_time = 0x0,
        .modified_date = 0x0,

        .cluster_high = start_cluster >> 16,
        .cluster_low = start_cluster,

        .filesize = request.buffer_size
    };
    driver_state.dir_table_buf.table[entry_row] = new_file;
    write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    // cluster not enough. FAT Table full
    if(cluster_needed > 0) return -1;
    
    return 0;
}

int8_t delete(__attribute__((unused)) struct FAT32DriverRequest request ){
    read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    // check if parent directory is not a directory
    if( !(driver_state.dir_table_buf.table->user_attribute == UATTR_NOT_EMPTY &&
          driver_state.dir_table_buf.table->attribute == ATTR_SUBDIRECTORY)){
        // not a directory
        return -1;
    }

    int i;
    bool isExist = false;
    bool isFolder = false;
    uint32_t cluster_number = 0;
    uint32_t entry_row = 0;
    // check if file name & ext exist
    for(i=2; i<64; i++){
        if(memcmp(driver_state.dir_table_buf.table[i].name, request.name, 8) == 0 && // check name
           memcmp(driver_state.dir_table_buf.table[i].ext, request.ext, 3) == 0){ // check ext
            isExist = true;

            // get file / folder location in Table Allication
            entry_row = i;

            // get the cluster_number in FAT Table
            uint16_t cluster_high = driver_state.dir_table_buf.table[i].cluster_high;
            uint16_t cluster_low = driver_state.dir_table_buf.table[i].cluster_low;
            cluster_number = (cluster_high << 16) + cluster_low;
            
            // check file / folder
            if(driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                isFolder = true;
            }
            break;
           }
    }

    // is file not exist
    if(!isExist) return 1;

    // handle kasus folder
    if(isFolder){
        // load directory to delete
        struct FAT32DirectoryTable target_dir;
        memset(target_dir.table, 0x00, sizeof(target_dir.table));
        read_clusters(target_dir.table, cluster_number, 1);

        // check if folder empty or not
        // --- cek definisi folder kosonglagi di spek
        bool isEmpty = true;
        int i;
        for(i=2; i<64; i++){
            if(target_dir.table[i].user_attribute == UATTR_NOT_EMPTY){
                isEmpty = false;
                break;
            }
        }

        // jika folder tidak kosong
        if(!isEmpty) return 2;

        // delete from parent table

        // hapus folder name
        memset(driver_state.dir_table_buf.table[entry_row].name, 0x00, sizeof(driver_state.dir_table_buf.table[entry_row].name));
        // hapus attribute (file type: file / folder)
        driver_state.dir_table_buf.table[entry_row].attribute = FAT32_FAT_EMPTY_ENTRY;
        // hapus user attribute
        driver_state.dir_table_buf.table[entry_row].user_attribute = FAT32_FAT_EMPTY_ENTRY;
        // write to table cluster
        write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

        // delete the folder itself
        memset(target_dir.table[0].name, 0x00, sizeof(driver_state.dir_table_buf.table[entry_row].name));
        memset(target_dir.table[1].name, 0x00, sizeof(driver_state.dir_table_buf.table[entry_row].name));
        // hapus attribute (file type: file / folder)
        target_dir.table[0].attribute = FAT32_FAT_EMPTY_ENTRY;
        target_dir.table[1].attribute = FAT32_FAT_EMPTY_ENTRY;
        // hapus user attribute
        target_dir.table[0].user_attribute = FAT32_FAT_EMPTY_ENTRY;
        target_dir.table[1].user_attribute = FAT32_FAT_EMPTY_ENTRY;
        // write to table cluster
        write_clusters(target_dir.table, cluster_number, 1);

        // delete from FAT
        driver_state.fat_table.cluster_map[cluster_number] = FAT32_FAT_EMPTY_ENTRY;
        write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

        // succeed
        return 0;
    }

    // handle kasus file

    /* REFACTORING */
    /*
    // hapus file name
    memset(driver_state.dir_table_buf.table[entry_row].name, 0x00, sizeof(driver_state.dir_table_buf.table[entry_row].name));
    // hapus extention
    memset(driver_state.dir_table_buf.table[entry_row].ext, 0x00, sizeof(driver_state.dir_table_buf.table[entry_row].ext));
    // hapus attribute (file type: file / folder)
    driver_state.dir_table_buf.table[entry_row].attribute = FAT32_FAT_EMPTY_ENTRY;
    // hapus user attribute
    driver_state.dir_table_buf.table[entry_row].user_attribute = FAT32_FAT_EMPTY_ENTRY;
    */
    memset(&driver_state.dir_table_buf.table[entry_row], 0x00, sizeof(driver_state.dir_table_buf.table[entry_row]));

    // write to table cluster
    write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);

    // delete file and all its cluster from FAT table
    uint32_t next_cluster = driver_state.fat_table.cluster_map[cluster_number];
    driver_state.fat_table.cluster_map[cluster_number] = FAT32_FAT_EMPTY_ENTRY;
    
    struct ClusterBuffer cluster_buffer;
    memset(cluster_buffer.buf, 0x0, CLUSTER_SIZE);
    write_clusters(cluster_buffer.buf, cluster_number, 1);

    while(next_cluster != FAT32_FAT_END_OF_FILE){
        cluster_number = next_cluster;
        next_cluster = driver_state.fat_table.cluster_map[cluster_number];
        driver_state.fat_table.cluster_map[cluster_number] = FAT32_FAT_EMPTY_ENTRY;

        write_clusters(cluster_buffer.buf, cluster_number, 1);
    }
    driver_state.fat_table.cluster_map[next_cluster] = FAT32_FAT_EMPTY_ENTRY;
    write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    write_clusters(cluster_buffer.buf, cluster_number, 1);
    
    // succeed
    return 0;
}