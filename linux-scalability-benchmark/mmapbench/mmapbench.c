/**
 * @file   mmap.c
 * @original author Wang Yuanxuan <zellux@gmail.com>
 * @modified by: sudarsun@gatech.edu
 * @date   Fri Jan  8 21:23:31 2010
 * @brief  An implementation of mmap bench mentioned in OSMark paper
 * 
 * 
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>

#include "config.h"
#include "bench.h"

int nbufs = 128000;
//int nbufs = 64000;
char *shared_area = NULL;
int flag[32];
int ncores = 1;
char *filename = "/mnt/pmfs/shared.dat";

void *
worker(void *args)
{
    int id = (long) args;
    int ret = 0;
    int i;

    affinity_set(id);

    for (i = 0; i < nbufs; i++)
        ret += shared_area[i *4096];
    
    //printf("potato_test: thread#%d done.\n", core);

    return (void *) (long) ret;
}

void* setup_map_file(char *filepath, unsigned long bytes) {
    int result;
    int fd = -1;
    void *addr = NULL;

    fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(-1);
    }
    ftruncate(fd, bytes);
    /*result = lseek(fd,bytes, SEEK_SET);
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(-1);
    }
    result = write(fd, "", 1);
    if (result != 1) {
        close(fd);
        perror("Error writing last byte of the file");
        exit(-1);
    }*/
	return fd;
}

int
main(int argc, char **argv)
{
    int i, fd;
    pthread_t tid[32];
    uint64_t start, end, usec;

    for (i = 0; i < ncores; i++) {
        flag[i] = 0;
    }

    if (argc > 1) {
        ncores = atoi(argv[1]);
    }

	fd = setup_map_file(filename, (nbufs+1)*4096);
    //fd = open(filename, O_RDONLY);
    shared_area = mmap(0, (1 + nbufs) * 4096, PROT_READ, MAP_PRIVATE, fd, 0);
    
    start = read_tsc();
    for (i = 0; i < ncores; i++) {
        pthread_create(&tid[i], NULL, worker, (void *) (long) i);
    }

    for (i = 0; i < ncores; i++) {
        pthread_join(tid[i], NULL);
    }
    
    end = read_tsc();
    usec = (end - start) * 1000000 / get_cpu_freq();
    printf("usec: %ld\t\n", usec);

    close(fd);
    return 0;
}
