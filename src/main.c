#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "fat32.h"
#include "disk.h"


int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    open_disk_image(argv[1]);


    read_BPB(file);
    read_EBPB(file);
    read_FAT(file, 0);


    print_BPB();
    print_EBPB();
    print_volume_info();


    init_sector(bpb.bytes_per_sector);
    init_cluster(bpb.sectors_per_cluster, bpb.bytes_per_sector * (bpb.reserved_sectors+bpb.number_of_fats*ebpb.sectors_per_FAT32));


    // printf("%lu\n", bpb.bytes_per_sector*(bpb.reserved_sectors+bpb.number_of_fats*ebpb.sectors_per_FAT32+get_cluster_size()*(2-2)));


    bool running = true;
    struct deque *current = NULL;
    create_deque(&current, 0);
    push_back_deque(&current, ebpb.root_cluster);


    while(running){
        putchar('/');
        for(size_t i=1; i<current->count; ++i){
            bool flag = false;
            struct deque *entries = get_entries(current->data[i - 1]);
            for(size_t j=0; j<entries->count; ++j){
                const struct entry *e = (struct entry *) entries->data[j];

                if(e->first_cluster == current->data[i]){
                    flag = true;
                    printf("%s/", (e->long_name == NULL ? e->short_name : e->long_name));

                    break;
                }
            }

            assert(flag);

            delete_entries(entries);
        }

        printf("> ");

        char line[400];
        memset(line, 0, sizeof(line));

        fgets(line, 400, stdin);
        if(line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';


        size_t number_of_tokens = 0;
        char **tokens = tokenize(line, " ", &number_of_tokens);


        if(!strcmp(tokens[0], "format")){
            assert(false);
        }
        else if(!strcmp(tokens[0], "ls")){
            assert(number_of_tokens==1 || number_of_tokens==2);

            struct deque *entries = NULL;
            if(number_of_tokens == 1){
                entries = get_entries(get_back_deque(&current));
            }
            else{
                struct deque *dq = NULL;
                create_deque(&dq, 0);

                if(tokens[1][0] == '/'){
                    push_back_deque(&dq, ebpb.root_cluster);
                }
                else{
                    delete_deque(&dq);
                    dq = make_copy_deque(&current);
                }

                solve(tokens[1], dq);

                entries = get_entries(get_back_deque(&dq));

                delete_deque(&dq);
            }

            print_entries(entries);

            delete_entries(entries);
        }
        else if(!strcmp(tokens[0], "cd")){
            assert(number_of_tokens == 2);

            struct deque *dq = NULL;
            create_deque(&dq, 0);

            if(tokens[1][0] == '/'){
                push_back_deque(&dq, ebpb.root_cluster);
            }
            else{
                delete_deque(&dq);
                dq = make_copy_deque(&current);
            }

            solve(tokens[1], dq);

            current = make_copy_deque(&dq);
            delete_deque(&dq);
        }
        else if(!strcmp(tokens[0], "touch")){
            assert(number_of_tokens == 2);
        }
        else if(!strcmp(tokens[0], "mkdir")){
            assert(number_of_tokens == 2);
        }
        else if(!strcmp(tokens[0], "exit")){
            running = false;
        }

        for(size_t i=0; i<number_of_tokens; ++i) free(tokens[i]);
        free(tokens);
    }


    delete_deque(&current);
    free(sector_buffer);
    free(cluster_buffer);
    free(name);
    free(fat);


    close_disk_image(file);


    return 0;
}