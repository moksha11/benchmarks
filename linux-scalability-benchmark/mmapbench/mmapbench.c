/**
 * @file   mmap.c
 * @author Wang Yuanxuan <zellux@gmail.com>
 * @date   Fri Jan  8 21:23:31 2010
 * 
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
#include <assert.h>

#include "config.h"
#include "bench.h"

#define USENVMALLOC
#ifdef USENVMALLOC
#include <nv_map.h>
#include <c_io.h>
#define __NR_nv_mmap_pgoff 314
#endif

#define PROCID 112
#define CHUNKID 112
#define NOPERSIST 1

//int nbufs = 256000;
int nbufs = 64000;
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

    for (i = 0; i < nbufs; i++){
		shared_area[i *4096]=1;
        ret += shared_area[i *4096];
	}
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
    result = lseek(fd,bytes, SEEK_SET);
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
    }
	return fd;
}

#ifdef USENVMALLOC
void* map_using_mmap(size_t size){

	char *map;
	int proc_id = PROCID;
    int chunk_id =CHUNKID;
    struct nvmap_arg_struct a;

	a.fd = -1;
    a.offset = 0;
    a.vma_id =chunk_id;
    a.proc_id = proc_id;
    a.pflags = 1;
    a.noPersist = NOPERSIST;
	map = (char *) syscall(__NR_nv_mmap_pgoff, 0, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE| MAP_ANONYMOUS, &a);
	//map = (char *) mmap(0, size, PROT_READ, MAP_PRIVATE| MAP_ANONYMOUS,-1, 0);
	return map;
}
#endif

int main(int argc, char **argv)
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

    if (argc > 2) {
        nbufs = atoi(argv[2]);
    }


#ifndef USENVMALLOC
	fd = setup_map_file(filename, (nbufs+1)*4096);
    //fd = open(filename, O_RDONLY);
    shared_area = mmap(0, (1 + nbufs) * 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
#else
	shared_area = map_using_mmap((1 + nbufs) * 4096);
#endif
	assert(shared_area != MAP_FAILED);
    
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
