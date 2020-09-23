#ifndef utilities_C
#define utilities_C

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "utilities.h"

void init_ipc() {
	shmid = shmget(SHM_KEY, sizeof(T_int_malloc_stats), IPC_CREAT | 0666);
	if(shmid == -1) {
		puts("ERR: IPC failed at shmget");
		exit(1);
	}

	shmem = (T_int_malloc_stats*) shmat(shmid, NULL, 0);
	if (shmem == (void *) -1) {
		puts("ERR: IPC failed at shmat");
		end_ipc();
		exit(1);
	}
	__atomic_sub_fetch(&(shmem->pid), 1, __ATOMIC_SEQ_CST);
}

void end_ipc() {
	if (shmdt(shmem) == -1) {
		puts("ERR: IPC failed at shmdt");
	}

	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		puts("ERR: IPC may have failed (ignore if \"Invalid Argument\") at shmctl");
	}
}

unsigned long long get_cur_time_ms(int print) {
    unsigned long long ms;
    time_t s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = spec.tv_nsec / 1.0e6;
    if (ms > 9999) {
        s++;
        ms = 0;
    }

    if(print) {
		printf("Current timestamp: %lu.%04llu seconds since the Epoch\n", s, ms);
    }

    ms = s * 1000 + ms;
    return ms;
}

T_memory_snapshot parse_proc_maps(char* fname) {
	FILE* fin = fopen(fname, "r");

	T_memory_snapshot mem_snapshot = {0};
	mem_snapshot.timestamp_ms = get_cur_time_ms(0);

	char buffer[1000];
	char* fgets_status = fgets(buffer, 999, fin);

	while(fgets_status != NULL) {
		char* addr_space_start = (char*) strtok(buffer, "-");
		char* addr_space_end = strtok(NULL, " ");
		unsigned long int bytes = (unsigned long int) (strtoul(addr_space_end, NULL, 16) - strtoul(addr_space_start, NULL, 16));

		// ignore next 4 coulumns in line
		char* owner = strtok(NULL, " ");
		owner = strtok(NULL, " ");
		owner = strtok(NULL, " ");
		owner = strtok(NULL, " ");
		owner = strtok(NULL, " ");

//		verbose_print("from proc: %lu %s", bytes, owner);
		if ((strtoul(addr_space_end, NULL, 16) <= 4294967295) && (owner[0] == '/')) {
			mem_snapshot.text += bytes;
		} else if (strcmp(owner, "[heap]\n") == 0) {
			mem_snapshot.heap += bytes;
		} else if (strcmp(owner, "\n") == 0) {
			mem_snapshot.mmap += bytes;
		} else if (owner[0] == '/') {
			mem_snapshot.mmap += bytes;
		} else if (strcmp(owner, "[stack]\n") == 0) {
			mem_snapshot.stack += bytes;
		} else if (strcmp(owner, "[vvar]\n") == 0) {
			mem_snapshot.vvar += bytes;
		} else if (strcmp(owner, "[vdso]\n") == 0) {
			mem_snapshot.vdso += bytes;
		} else if (memcmp(owner, "[vsyscall]", 10) == 0) {
			mem_snapshot.vsyscall += bytes;
		} else {
			mem_snapshot.unfigured += bytes;
		}

		fgets_status = fgets(buffer, 1000, fin);
	}

    mem_snapshot.total_dynamic = mem_snapshot.heap + mem_snapshot.mmap + mem_snapshot.mmap_so;
    fclose(fin);
	return mem_snapshot;
}

#endif