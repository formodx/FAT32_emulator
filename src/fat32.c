#include "fat32.h"


char *name;
uint32_t *fat;
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


// void format(FILE *file){
//     struct BPB bpb;
//     bpb.jump_code[0] = 0xEB;
//     bpb.jump_code[1] = 0x3C;
//     bpb.jump_code[2] = 0x90;
//     memcpy(bpb.oem_name, "mkfs.fat", strlen("mkfs.fat"));
//     bpb.bytes_per_sector = 512;
//     bpb.sectors_per_cluster = 1;
//     bpb.reserved_sectors = 32;
//     bpb.number_of_fats = 2;
//     bpb.root_entries = 0;
//     bpb.total_sectors_16 = 0;
//     bpb.media_descriptor = 0xF8;
//     bpb.sectors_per_FAT16 = 0;
//     bpb.sectors_per_track = 32;
//     bpb.number_of_heads;
//     bpb.hidden_sectors = 0;
//     bpb.total_sectors_32;

//     assert(bpb.bytes_per_sector*bpb.sectors_per_cluster <= 32*1024);
//     assert(bpb.reserved_sectors);
//     assert(bpb.number_of_fats == 2);
//     assert(!bpb.root_entries);
//     assert(!bpb.total_sectors_16);
//     assert(!bpb.sectors_per_FAT16);
//     assert(bpb.total_sectors_32);

//     struct EBPB ebpb;
//     // ebpb.sectors_per_FAT32 = ;
//     ebpb.root_cluster = 2;
//     ebpb.info_sector = 1;
//     ebpb.backup_boot_sector = 6;
//     for(int i=0; i<12; ++i) ebpb.reserved[i] = 0;
//     // memset(ebpb.reserved, 0, 12);

//     ebpb.vi.drive_number = 0x80;
//     ebpb.vi.flags = 0;
//     ebpb.vi.signature = 0x29;
//     // ebpb.vi.volume_id;
//     memcpy(ebpb.vi.volume_label, "NO NAME    ", strlen("NO NAME    "));
//     memcpy(ebpb.vi.system_identifier, "FAT32   ", strlen("FAT32   "));

//     ebpb.bootable_partition_signature = 0xAA55;
// }


struct deque *get_clusters(int cluster){
    struct deque *dq = NULL;
    create_deque(&dq, 0);

    while(cluster < 0x0FFFFFF8){
        push_back_deque(&dq, cluster);
        cluster = fat[cluster];
    }

    return dq;
}


char *get_string_from_long_entry(const struct long_directory_entry *long_entry){
    int indexes[] = {
        1, 3, 5, 7, 9,
        14, 16, 18, 20, 22, 24,
        28, 30
    };

    uint8_t *ptr = (void *) long_entry;
    size_t cnt = 0;
    for(int i=0; i<13 && *((char16_t *) (ptr+indexes[i])); ++i) ++cnt;

    char16_t *s = malloc(sizeof(char16_t) * (cnt+1));
    for(size_t i=0; i<cnt; ++i) s[i] = *((char16_t *) (ptr+indexes[i]));
    s[cnt] = u'\0';

    char *result = malloc(sizeof(char) * 128);
    memset(result, 0, 128);

    char *inbuf = (char *) s, *outbuf = result;
    size_t inbytesleft = sizeof(char16_t) * (cnt+1), outbytesleft = 128;

    iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
    if(cd == (iconv_t) -1){
        perror("iconv_open");
        exit(EXIT_FAILURE);
    }

    if(iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t) -1){
        perror("iconv");
        exit(EXIT_FAILURE);
    }

    if(iconv_close(cd) == -1){
        perror("iconv_close");
        exit(EXIT_FAILURE);
    }

    free(s);

    return result;
}


bool read_directory_entry(uint8_t **ptr, struct deque **clusters){
    if(**ptr == 0x00) return false;
    if(**ptr == 0xE5){
        *ptr += 32;

        return true;
    }

    free(name);
    name = NULL;

    size_t bytes_per_cluster = get_sector_size() * get_cluster_size();

    assert(*ptr-cluster_buffer >= 0);
    assert(*ptr-cluster_buffer < bytes_per_cluster);

    struct deque *dq = NULL;
    create_deque(&dq, 0);

    const struct long_directory_entry *entry = NULL;
    for(;;){
        if(*ptr-cluster_buffer >= bytes_per_cluster){
            if((entry->order & LAST_LONG_ENTRY-1) == 1) break;

            read_cluster(get_front_deque(clusters));
            pop_front_deque(clusters);

            *ptr = cluster_buffer;
        }

        entry = (struct long_directory_entry *) *ptr;

        if(entry->attributes != ATTR_LONG_NAME) break;

        char *s = get_string_from_long_entry(entry);
        for(int i=strlen(s)-1; i>=0; --i) push_front_deque(&dq, s[i]);

        free(s);

        *ptr += 32;
    }

    memcpy(&short_entry, *ptr, sizeof(struct short_directory_entry));

    if(!short_entry.first_cluster_high && !short_entry.first_cluster_low) short_entry.first_cluster_low = 2;

    if(dq->count){
        name = malloc(sizeof(char) * (dq->count+1));
        for(size_t i=0; i<dq->count; ++i) name[i] = dq->data[i];
        name[dq->count] = '\0';
    }

    delete_deque(&dq);

    *ptr += 32;

    return true;
}


struct deque *get_entries(int cluster){
    struct deque *clusters = get_clusters(cluster);
    struct deque *entries = NULL;
    create_deque(&entries, 0);

    while(clusters->count){
        read_cluster(get_front_deque(&clusters));
        pop_front_deque(&clusters);

        uint8_t *ptr = cluster_buffer;
        while(read_directory_entry(&ptr, &clusters)){
            if(*ptr == 0xE5) continue;

            struct entry *e = malloc(sizeof(struct entry));
            e->attributes = short_entry.attributes;
            e->size = short_entry.size;
            e->first_cluster = (short_entry.first_cluster_high<<16) | short_entry.first_cluster_low;

            char *temp1 = malloc(sizeof(char) * 12);
            strncpy(temp1, short_entry.name, 11);
            temp1[11] = '\0';

            e->short_name = temp1;

            if(name != NULL){
                char *temp2 = malloc(sizeof(char) * (strlen(name)+1));
                strcpy(temp2, name);
                temp2[strlen(name)] = '\0';

                e->long_name = temp2;

                free(name);
                name = NULL;
            }

            push_back_deque(&entries, (uint64_t) e);
        }
    }

    delete_deque(&clusters);

    return entries;
}


void delete_entries(struct deque *entries){
    assert(entries != NULL);

    for(size_t i=0; i<entries->count; ++i){
        struct entry *e = (struct entry *) entries->data[i];

        free(e->short_name);
        free(e->long_name);
        free(e);
    }

    delete_deque(&entries);
}


char *make_short_name(const char *s){
    char *result = malloc(sizeof(char) * 12);
    memset(result, ' ', 11);
    result[11] = '\0';

    if(!strcmp(s, ".")){
        result[0] = '.';

        return result;
    }

    if(!strcmp(s, "..")){
        result[0] = '.';
        result[1] = '.';

        return result;
    }

    size_t name_number_of_tokens = 0;
    char **name_tokens = tokenize(s, ".", &name_number_of_tokens);

    if(name_number_of_tokens!=1 && name_number_of_tokens!=2) return NULL;
    assert(name_number_of_tokens==1 || name_number_of_tokens==2);

    if(name_number_of_tokens == 1){
        if(strlen(name_tokens[0]) >= 12) return NULL;
        assert(strlen(name_tokens[0]) <= 11);

        for(size_t i=0; i<strlen(name_tokens[0]); ++i) result[i] = toupper(name_tokens[0][i]);
    }
    else{
        if(strlen(name_tokens[0]) >= 9) return NULL;
        if(strlen(name_tokens[1]) >= 4) return NULL;

        assert(strlen(name_tokens[0]) <= 8);
        assert(strlen(name_tokens[1]) <= 3);

        for(size_t i=0; i<strlen(name_tokens[0]); ++i) result[i] = toupper(name_tokens[0][i]);
        for(size_t i=0; i<strlen(name_tokens[1]); ++i) result[i+8] = toupper(name_tokens[1][i]);
    }

    result[11] = '\0';

    return result;
}


void create_directory_entry(){
}


void print_volume_info(){
    printf("Drive number: 0x%X\n", ebpb.vi.drive_number);
    printf("Flags: 0x%X\n", ebpb.vi.flags);
    printf("Signature: 0x%X\n", ebpb.vi.signature);
    printf("Volume ID 'Serial' number: 0x%X\n", ebpb.vi.volume_id);
    printf("Volume label string: '%.11s'\n", ebpb.vi.volume_label);
    printf("System identifier string: '%.8s'\n", ebpb.vi.system_identifier);
}


void print_BPB(){
    printf("OEM identifier: %8s\n", bpb.oem_name);
    printf("The number of bytes per sector: %hu\n", bpb.bytes_per_sector);
    printf("Number of sectors per cluster: %hhu\n", bpb.sectors_per_cluster);
    printf("Number of reserved sectors: %hu\n", bpb.reserved_sectors);
    printf("Number of File Allocation Tables: %hhu\n", bpb.number_of_fats);
    printf("Number of root directory entries: %hu\n", bpb.root_entries);
    printf("The total sectors in the logical volume: %hu\n", bpb.total_sectors_16);
    printf("The media descriptor type: 0x%X\n", bpb.media_descriptor);
    printf("Number of sectors per FAT. FAT12/FAT16 only: %hu\n", bpb.sectors_per_FAT16);
    printf("Number of sectors per track: %hu\n", bpb.sectors_per_track);
    printf("Number of heads or sides on the storage media: %hu\n", bpb.number_of_heads);
    printf("Number of hidden sectors: %u\n", bpb.hidden_sectors);
    printf("Large sector count: %u\n", bpb.total_sectors_32);
}


void print_EBPB(){
    printf("Sectors per FAT: %u\n", ebpb.sectors_per_FAT32);
    printf("The cluster number of the root directory: %u\n", ebpb.root_cluster);
    printf("The sector number of the FSInfo structure: %hu\n", ebpb.info_sector);
    printf("The sector number of the backup boot sector: %hu\n", ebpb.backup_boot_sector);
    printf("Bootable partition signature: 0x%X\n", ebpb.bootable_partition_signature);
}


void print_short_entry(const struct short_directory_entry *short_entry){
    assert(short_entry->name[0] != 0x00);
    assert(short_entry->name[0] != 0xE5);

    printf("name: %.11s\n", short_entry->name);
    printf("attributes: %X\n", short_entry->attributes);
    printf("first_cluster_high: %04X\n", short_entry->first_cluster_high);
    printf("first_cluster_low: %04X\n", short_entry->first_cluster_low);
    printf("size: %u\n", short_entry->size);
}


void print_long_entry(const struct long_directory_entry *long_entry){
    assert(long_entry->attributes == ATTR_LONG_NAME);

    printf("order: 0x%02X\n", long_entry->order);
    printf("name1:");
    for(int i=0; i<5; ++i) printf(" 0x%04X", long_entry->name1[i]);
    putwchar(L'\n');
    printf("attributes: 0x%02X\n", long_entry->attributes);
    printf("type: 0x%02X\n", long_entry->type);
    printf("checksum: %d\n", long_entry->checksum);
    printf("name2:");
    for(int i=0; i<6; ++i) printf(" 0x%04X", long_entry->name2[i]);
    putwchar(L'\n');
    printf("name3:");
    for(int i=0; i<2; ++i) printf(" 0x%04X", long_entry->name3[i]);
    putwchar(L'\n');
}


void print_entries(const struct deque *entries){
    assert(entries != NULL);

    printf("attributes | size | first_cluster |  short_name | long_name\n");
    printf("-----------------------------------------------------------\n");

    for(size_t i=0; i<entries->count; ++i){
        const struct entry *e = (struct entry *) entries->data[i];

        printf("%10hhu | %4u | %13u | %s |", e->attributes, e->size, e->first_cluster, e->short_name);

        if(e->long_name != NULL) printf(" %s", e->long_name);
        else printf("NULL");

        putchar('\n');
    }
}