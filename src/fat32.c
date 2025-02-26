#include "fat32.h"


char16_t *name;
uint32_t *fat;
struct deque entries;
struct deque clusters;
struct BPB bpb;
struct EBPB ebpb;
struct short_directory_entry short_entry;


void read_BPB(FILE *file){
    fseek(file, 0L, SEEK_SET);
    fread(&bpb, 1, sizeof bpb, file);
    fseek(file, 0L, SEEK_SET);
}


void read_EBPB(FILE *file){
    fseek(file, sizeof bpb, SEEK_SET);
    fread(&ebpb, 1, sizeof ebpb, file);
    fseek(file, 0L, SEEK_SET);

    assert(ebpb.bootable_partition_signature == 0xAA55);
}


void read_FAT(FILE *file, int idx){
    if(idx >= bpb.number_of_fats){
        fprintf(stderr, "wrong index\n");
        exit(EXIT_FAILURE);
    }

    fat = malloc(bpb.bytes_per_sector * ebpb.sectors_per_FAT32);
    memset(fat, 0, bpb.bytes_per_sector * ebpb.sectors_per_FAT32);

    fseek(file, bpb.bytes_per_sector * (bpb.reserved_sectors+ebpb.sectors_per_FAT32*idx), SEEK_SET);
    fread(fat, bpb.bytes_per_sector, ebpb.sectors_per_FAT32, file);
    fseek(file, 0L, SEEK_SET);
}


void get_clusters(int cluster){
    delete_deque(&clusters);
    create_deque(&clusters, 0);

    while(cluster < 0x0FFFFFF8){
        push_back_deque(&clusters, cluster);
        cluster = fat[cluster];
    }
}


const char16_t *get_string_from_long_entry(struct long_directory_entry *entry){
    uint8_t *ptr = (void *) entry;
    size_t cnt = 0, indexes[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};
    for(size_t i=0; ptr[indexes[i]] && i<13; ++i) ++cnt;

    // wprintf(L"name1: %ls\n", convert_to_wide(entry->name1, 5));
    // wprintf(L"name2: %ls\n", convert_to_wide(entry->name2, 6));
    // wprintf(L"name3: %ls\n", convert_to_wide(entry->name3, 2));

    char16_t *str = malloc(sizeof(char16_t) * (cnt+1));
    for(size_t i=0; i<cnt; ++i) str[i] = *((char16_t *) (ptr+indexes[i]));
    str[cnt] = u'\0';

    // wprintf(L"str: %ls\n", convert_to_wide(str, cnt));

    return str;
}


void make_entry(struct entry *e, char *sname, wchar_t *lname){
    assert(sname != NULL);

    char *temp1 = malloc(sizeof(char) * 12);
    strncpy(temp1, sname, 11);
    temp1[11] = '\0';

    wchar_t *temp2 = NULL;
    if(lname != NULL){
        temp2 = malloc(sizeof(wchar_t) * (wcslen(lname)+1));
        wcscpy(temp2, lname);
    }

    e->short_name = temp1;
    e->long_name = temp2;
}


bool read_directory_entry(uint8_t **ptr){
    assert((*ptr)-cluster_buffer < get_sector_size()*get_cluster_size());

    if(**ptr == 0x00) return false;
    if(**ptr == 0xE5){
        *ptr += 32;

        return true;
    }

    // assert(**ptr & LAST_LONG_ENTRY);

    struct long_directory_entry *entry = NULL;

    struct deque dq;
    create_deque(&dq, 0);
    for(;;){
        if(*ptr-cluster_buffer >= get_sector_size()*get_cluster_size()){
            if((entry->order & LAST_LONG_ENTRY-1) == 1) break;

            read_cluster(get_front_deque(&clusters));
            pop_front_deque(&clusters);

            *ptr = cluster_buffer;
        }

        entry = (struct long_directory_entry *) *ptr;

        if(entry->attributes != ATTR_LONG_NAME) break;

        // print_long_entry(entry);
        // putwchar(L'\n');

        const char16_t *s = get_string_from_long_entry(entry);
        for(int i=utf16_strlen(s)-1; i>=0; --i) push_front_deque(&dq, s[i]);

        *ptr += 32;
    }

    memcpy(&short_entry, *ptr, sizeof(struct short_directory_entry));

    if(!short_entry.first_cluster_high && !short_entry.first_cluster_low) short_entry.first_cluster_low = 2;

    if(dq.count){
        size_t length = dq.count;
        name = malloc(sizeof(char16_t) * (dq.count+1));
        for(size_t i=0; dq.count; ++i) name[i] = get_front_deque(&dq), pop_front_deque(&dq);
        name[length] = u'\0';
    }
    else{
        name = NULL;
    }

    // print_short_entry(&short_entry);

    *ptr += 32;

    return true;
}


void list_directory(int cluster){
    for(size_t i=0; i<entries.count; ++i){
        struct entry *e = (struct entry *) entries.data[i];

        free(e->short_name);
        free(e->long_name);
        free(e);
    }

    delete_deque(&entries);
    create_deque(&entries, 0);

    get_clusters(cluster);
    while(clusters.count){
        read_cluster(get_front_deque(&clusters));
        pop_front_deque(&clusters);

        uint8_t *ptr = cluster_buffer;
        while(read_directory_entry(&ptr)){
            if(*ptr == 0xE5) continue;

            struct entry *e = malloc(sizeof(struct entry));
            e->attributes = short_entry.attributes;
            e->size = short_entry.size;
            e->first_cluster = (short_entry.first_cluster_high<<16) | short_entry.first_cluster_low;

            if(name == NULL){
                make_entry(e, short_entry.name, NULL);
            }
            else{
                wchar_t *str = convert_to_wide(name, utf16_strlen(name));

                make_entry(e, short_entry.name, str);

                free(str);
            }

            push_back_deque(&entries, (uint64_t) e);

            // print_short_entry(&short_entry);
        }
    }
}


char *make_short_name(wchar_t *s){
    char *result = malloc(sizeof(char) * 12);
    memset(result, ' ', 11);
    result[11] = '\0';

    if(!wcscmp(s, L".")){
        result[0] = '.';

        return result;
    }

    if(!wcscmp(s, L"..")){
        result[0] = '.';
        result[1] = '.';

        return result;
    }

    size_t name_number_of_tokens = 0;
    wchar_t **name_tokens = tokenize(s, L".", &name_number_of_tokens);

    // wprintf(L":%ls\n", s);

    // wprintf(L"%lu\n", name_number_of_tokens);
    // for(size_t i=0; i<name_number_of_tokens; ++i){
    //     wprintf(L"%ls\n", name_tokens[i]);
    // }

    if(name_number_of_tokens!=1 && name_number_of_tokens!=2) return NULL;

    assert(name_number_of_tokens==1 || name_number_of_tokens==2);

    if(name_number_of_tokens == 1){
        if(wcslen(name_tokens[0]) > 11) return NULL;

        assert(wcslen(name_tokens[0]) < 12);

        for(size_t i=0; i<wcslen(name_tokens[0]); ++i) result[i] = toupper(name_tokens[0][i]);
    }
    else{
        if(wcslen(name_tokens[0]) > 8) return NULL;
        if(wcslen(name_tokens[1]) > 3) return NULL;

        assert(wcslen(name_tokens[0]) < 9);
        assert(wcslen(name_tokens[1]) < 4);

        for(size_t i=0; i<wcslen(name_tokens[0]); ++i) result[i] = toupper(name_tokens[0][i]);
        for(size_t i=0; i<wcslen(name_tokens[1]); ++i) result[i+8] = toupper(name_tokens[1][i]);
    }

    result[11] = '\0';

    return result;
}


void print_volume_info(){
    wprintf(L"Drive number: 0x%X\n", ebpb.vi.drive_number);
    wprintf(L"Signature: 0x%X\n", ebpb.vi.signature);
    // printf("Volume ID 'Serial' number:\n", vi->volume_id);
    wprintf(L"Volume label string: %.11s\n", ebpb.vi.volume_label);
    wprintf(L"System identifier string: %.8s\n", ebpb.vi.system_identifier);
}


void print_BPB(){
    wprintf(L"OEM identifier: %8s\n", bpb.oem_name);
    wprintf(L"The number of bytes per sector: %hu\n", bpb.bytes_per_sector);
    wprintf(L"Number of sectors per cluster: %hhu\n", bpb.sectors_per_cluster);
    wprintf(L"Number of reserved sectors: %hu\n", bpb.reserved_sectors);
    wprintf(L"Number of File Allocation Tables: %hhu\n", bpb.number_of_fats);
    wprintf(L"Number of root directory entries: %hu\n", bpb.root_entries);
    wprintf(L"The total sectors in the logical volume: %hu\n", bpb.total_sectors_16);
    wprintf(L"The media descriptor type: 0x%X\n", bpb.media_descriptor);
    wprintf(L"Number of sectors per FAT. FAT12/FAT16 only: %hu\n", bpb.sectors_per_FAT16);
    wprintf(L"Number of sectors per track: %hu\n", bpb.sectors_per_track);
    wprintf(L"Number of heads or sides on the storage media: %hu\n", bpb.number_of_heads);
    wprintf(L"Number of hidden sectors: %u\n", bpb.hidden_sectors);
    wprintf(L"Large sector count: %u\n", bpb.total_sectors_32);
}


void print_EBPB(){
    wprintf(L"Sectors per FAT: %u\n", ebpb.sectors_per_FAT32);
    wprintf(L"The cluster number of the root directory: %u\n", ebpb.root_cluster);
    wprintf(L"The sector number of the FSInfo structure: %hu\n", ebpb.info_sector);
    wprintf(L"The sector number of the backup boot sector: %hu\n", ebpb.backup_boot_sector);
    wprintf(L"Bootable partition signature: 0x%X\n", ebpb.bootable_partition_signature);
}


void print_short_entry(const struct short_directory_entry *entry){
    assert(entry->name[0] != 0x00);
    assert(entry->name[0] != 0xE5);

    wprintf(L"name: %.11s\n", entry->name);
    wprintf(L"attributes: %X\n", entry->attributes);
    wprintf(L"first_cluster_high: %04X\n", entry->first_cluster_high);
    wprintf(L"first_cluster_low: %04X\n", entry->first_cluster_low);
    wprintf(L"size: %u\n", entry->size);
}


void print_long_entry(const struct long_directory_entry *entry){
    wprintf(L"order: 0x%02X\n", entry->order);
    wprintf(L"name1: %ls\n", convert_to_wide(entry->name1, 5));
    wprintf(L"attributes: 0x%02X\n", entry->attributes);
    wprintf(L"type: 0x%02X\n", entry->type);
    wprintf(L"checksum: %d\n", entry->checksum);
    wprintf(L"name2: %ls\n", convert_to_wide(entry->name2, 6));
    wprintf(L"name3: %ls\n", convert_to_wide(entry->name3, 2));
}


void print_entries(){
    for(size_t i=0; i<entries.count; ++i){
        const struct entry *e = (struct entry *) entries.data[i];

        wprintf(L"%3hhu %10u %10u ", e->attributes, e->size, e->first_cluster);

        if(e->long_name == NULL){
            wprintf(L"%.11s\n", e->short_name);
        }
        else{
            wprintf(L"%ls\n", e->long_name);
        }
    }
}