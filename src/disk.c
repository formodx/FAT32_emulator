#include "disk.h"


uint8_t *sector_buffer = NULL, *cluster_buffer = NULL;
static size_t sector_size, cluster_size, cluster_offset;
FILE *file = NULL;


void open_disk_image(const char *filename){
    file = fopen(filename, "rb");
    if(file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}


void close_disk_image(){
    if(fclose(file) == EOF){
        perror("fclose");
        exit(EXIT_FAILURE);
    }
}


void init_sector(size_t size){
    sector_buffer = malloc(size);
    sector_size = size;
}


void read_sector(int sector){
    if(!sector_size){
        fprintf(stderr, "call init_sector() before using read_sector()\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, sector_size * sector, SEEK_SET);
    fread(sector_buffer, 1, sector_size, file);
}


size_t get_sector_size(){
    if(!sector_size){
        fprintf(stderr, "call init_sector() before using read_sector()\n");
        exit(EXIT_FAILURE);
    }

    return sector_size;
}


void init_cluster(size_t size, int offset){
    cluster_buffer = malloc(sector_size * size);
    cluster_size = size;
    cluster_offset = offset;
}


void read_cluster(int cluster){
    if(!sector_size){
        fprintf(stderr, "call init_sector() before using read_cluster()\n");
        exit(EXIT_FAILURE);
    }

    if(!cluster_size){
        fprintf(stderr, "call init_cluster() before using read_cluster()\n");
        exit(EXIT_FAILURE);
    }

    if(cluster < 2){
        fprintf(stderr, "wrong cluster\n");
        exit(EXIT_FAILURE);
    }

    for(size_t i=0; i<cluster_size; ++i){
        read_sector(cluster_offset/sector_size + cluster_size*(cluster-2) + i);

        memcpy(cluster_buffer + sector_size*i, sector_buffer, sector_size);
    }
}


size_t get_cluster_size(){
    if(!cluster_size){
        fprintf(stderr, "call init_cluster() before using read_cluster()\n");
        exit(EXIT_FAILURE);
    }

    return cluster_size;
}