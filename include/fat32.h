#ifndef FAT32_H
#define FAT32_H


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <iconv.h>
#include <uchar.h>
#include "disk.h"
#include "deque.h"
#include "utils.h"


#define BOOTCODE_FAT32_SIZE 420
#define LAST_LONG_ENTRY 0x40

#define ATTR_READ_ONLY (0x01)
#define ATTR_HIDDEN (0x02)
#define ATTR_SYSTEM (0x04)
#define ATTR_VOLUME_ID (0x08)
#define ATTR_DIRECTORY (0x10)
#define ATTR_ARCHIVE (0x20)
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)

// BAD_CLUSTER
// #define BAD (0x0FFFFFF7)
// #define EOC (0x0FFFFFFF)


struct volume_info{
    uint8_t drive_number;
    uint8_t flags;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char system_identifier[8];
} __attribute__((packed));


struct BPB{
    uint8_t jump_code[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t sectors_per_FAT16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
} __attribute__((packed));


struct EBPB{
    uint32_t sectors_per_FAT32;
    uint16_t flags;
    uint8_t version[2];
    uint32_t root_cluster;
    uint16_t info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    struct volume_info vi;
    uint8_t boot_code[BOOTCODE_FAT32_SIZE];
    uint16_t bootable_partition_signature;
} __attribute__((packed));


struct short_directory_entry{
    char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t modification_time;
    uint16_t modification_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed));


struct long_directory_entry{
    uint8_t order;
    char16_t name1[5];
    uint8_t attributes;
    uint8_t type;
    uint8_t checksum;
    char16_t name2[6];
    uint16_t first_clu;
    char16_t name3[2];
} __attribute__((packed));


struct entry{
    char *short_name;
    char *long_name;
    uint8_t attributes;
    uint32_t size;
    uint32_t first_cluster;
};


extern char *name;
extern uint32_t *fat;
extern struct BPB bpb;
extern struct EBPB ebpb;
extern struct short_directory_entry short_entry;


void read_BPB(FILE *);
void read_EBPB(FILE *);
void read_FAT(FILE *, int);
// void format(FILE *);
struct deque *get_clusters(int);
char *get_string_from_long_entry(const struct long_directory_entry *);
bool read_directory_entry(uint8_t **, struct deque **);
struct deque *get_entries(int);
void delete_entries(struct deque *);
char *make_short_name(const char *);
void create_directory_entry();
void print_volume_info();
void print_BPB();
void print_EBPB();
void print_short_entry(const struct short_directory_entry *);
void print_long_entry(const struct long_directory_entry *);
void print_entries(const struct deque *);


#endif