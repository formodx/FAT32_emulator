#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #include <locale.h>
#include <wchar.h>
#include "fat32.h"
#include "disk.h"


int main(int argc, char *argv[]){
    // setlocale(LC_ALL, "");


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


    // wprintf(L"%d\n", bpb.bytes_per_sector*(bpb.reserved_sectors+bpb.number_of_fats*ebpb.sectors_per_FAT32+get_cluster_size()*(2-2)));


    struct deque current;
    create_deque(&current, 0);
    push_back_deque(&current, ebpb.root_cluster);
    while(true){
        wprintf(L"/");
        for(int i=1; i<current.count; ++i){
            list_directory(current.data[i - 1]);

            bool flag = false;
            for(size_t j=0; j<entries.count; ++j){
                struct entry *e = (struct entry *) entries.data[j];

                if(e->first_cluster == current.data[i]){
                    flag = true;

                    if(e->long_name == NULL){
                        wprintf(L"%s", e->short_name);
                    }
                    else{
                        wprintf(L"%ls/", e->long_name);
                    }

                    break;
                }
            }
        }

        wprintf(L"> ");

        wchar_t line[400];
        memset(line, 0, sizeof(line));

        fgetws(line, 400, stdin);
        if(line[wcslen(line)-1] == L'\n') line[wcslen(line)-1] = L'\0';


        size_t number_of_tokens = 0;
        wchar_t **tokens = tokenize(line, L" ", &number_of_tokens);


        if(!wcscmp(tokens[0], L"ls")){
            assert(number_of_tokens==1 || number_of_tokens==2);

            if(number_of_tokens == 1){
                list_directory(get_back_deque(&current));
            }
            else{
                struct deque dq;
                create_deque(&dq, 0);

                if(tokens[1][0] == L'/'){
                    push_back_deque(&dq, ebpb.root_cluster);
                }
                else{
                    delete_deque(&dq);
                    dq = *make_copy_deque(&current);
                }

                solve(tokens[1], &dq);

                list_directory(get_back_deque(&dq));

                delete_deque(&dq);
            }

            print_entries();
        }
        else if(!wcscmp(tokens[0], L"cd")){
            assert(number_of_tokens == 2);

            struct deque dq;
            create_deque(&dq, 0);

            if(tokens[1][0] == L'/'){
                push_back_deque(&dq, ebpb.root_cluster);
            }
            else{
                dq = *make_copy_deque(&current);
            }

            solve(tokens[1], &dq);

            current = *make_copy_deque(&dq);
            delete_deque(&dq);
        }
        else if(!wcscmp(tokens[0], L"exit")){
            break;
        }

        for(size_t i=0; i<number_of_tokens; ++i) free(tokens[i]);
        free(tokens);
    }


    free(sector_buffer);
    free(cluster_buffer);
    free(name);
    free(fat);


    close_disk_image(file);


    return 0;
}