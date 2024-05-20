#ifndef _USER_SHELL
#define _USER_SHELL

#include "std/string.h"
#include "filesystem/fat32.h"

#define DEFAULT_ARGS_COUNT 10

/* CURSOR STATE */
typedef struct {
    int32_t row;
    int32_t col;
} CP;

/**
 * Extra Attribute for PUTS
 * 
 * Biar bisa passing cursor_pointer, daripada nambah param edi esi
 */
typedef struct {
    uint32_t count;
    uint32_t text_color;
} putsExtraAttribute;

typedef struct {
    char info[8];
    char ext[3];
    uint32_t len_info;
    uint8_t len_ext;
} Arg;

typedef struct {
    char info[2048];
    uint32_t len_info;
    uint32_t argument_neff;
} CommandStruct;

typedef struct {
    Arg path[10];
    uint32_t neff;
} CurrentWorkingDirectory;

/* Global var used */
extern CP cursor_position;
extern struct FAT32DriverState current_dir;
extern CurrentWorkingDirectory current_working_directory;

/* DEFINE COLOR BE USED */
#define BIOS_BLACK          0x0
#define BIOS_BLUE           0x1
#define BIOS_GREEN          0x2
#define BIOS_CYAN           0x3
#define BIOS_RED            0x4
#define BIOS_MAGENTA        0x5
#define BIOS_BROWN          0x6
#define BIOS_LIGHT_GRAY     0x7

#define BIOS_DARK_GRAY      0x8
#define BIOS_LIGHT_BLUE     0x9
#define BIOS_LIGHT_GREEN    0xA
#define BIOS_LIGHT_CYAN     0xB
#define BIOS_LIGHT_RED      0xC
#define BIOS_LIGHT_MAGENTA  0xD
#define BIOS_LIGHT_YELLOW   0xE
#define BIOS_WHITE          0xF

/* FUNCITON LIST */
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

uint32_t get_entry_row(Arg *argument);

uint32_t get_cluster_number();

uint32_t get_parent_cluster_number();

uint32_t get_cluster_number_self(struct FAT32DirectoryEntry *p);

#endif