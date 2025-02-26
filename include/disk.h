#ifndef DISK_H
#define DISK_H


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


extern uint8_t *sector_buffer, *cluster_buffer;
extern FILE *file;


void open_disk_image(const char *);
void close_disk_image();
void init_sector(size_t);
void read_sector(int);
size_t get_sector_size();
void init_cluster(size_t, int);
void read_cluster(int);
size_t get_cluster_size();


#endif