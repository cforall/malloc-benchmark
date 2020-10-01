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

T_memory_snapshot parse_proc_maps(const int pid) {
	char proc_maps_fname[500] = "/proc/self/maps\0";
	char proc_exe_fname[500] = "/proc/self/exe\0";
	char proc_exe_path[500];
	if (pid > 0) {
		sprintf(&proc_maps_fname[0], "/proc/%d/maps", pid);
		sprintf(&proc_exe_fname[0], "/proc/%d/exe", pid);
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
	realpath(&proc_exe_fname[0], &proc_exe_path[0]);
#pragma GCC diagnostic pop
	size_t proc_exe_path_len = strlen(proc_exe_path);

	FILE* fin = fopen(proc_maps_fname, "r");
	T_memory_snapshot mem_snapshot = {0};
	mem_snapshot.timestamp_ms = get_cur_time_ms(0);

	char buffer[1000];
	char* fgets_status = fgets(buffer, 999, fin);
	unsigned long int heap_end = 0;

	while(fgets_status) {
		char* strtok_state = NULL;
		char* addr_space_start = strtok_r(buffer, "-", &strtok_state); // start address
		char* addr_space_end = strtok_r(NULL, " ", &strtok_state); // end address
		unsigned long int bytes = (unsigned long int) (strtoul(addr_space_end, NULL, 16) - strtoul(addr_space_start, NULL, 16));

		char file_flag = '0'; // 0: non-file mapping, 1: file mapping
		char mapping_flag = 'u'; // t: text, h: heap, m: mmap, M: mmap_so(file), s: stack, v: vdso, V: vsyscall, w: vvar, u: unfigured // main use: debugging

		// ignore next 4 coulumns in line
		char* owner = strtok_r(NULL, " ", &strtok_state); // permissions
		owner = strtok_r(NULL, " ", &strtok_state); // offset
		owner = strtok_r(NULL, " ", &strtok_state); // dev
		owner = strtok_r(NULL, " ", &strtok_state); // file number
		if (owner[0] != '0') file_flag = '1';
		owner = strtok_r(NULL, " ", &strtok_state); // owner

		if (file_flag == '1') { // mapped file? either executable or dynamic loading
			if (strlen(owner) >= proc_exe_path_len && memcmp(owner, proc_exe_path, proc_exe_path_len) == 0) {
				mapping_flag = 't';
				mem_snapshot.text += bytes;
			} else {
				mapping_flag = 'M';
				mem_snapshot.mmap_so += bytes;
			}
		} else {
			if (owner[0] == '\n') {
				if (strtoul(addr_space_start, NULL, 16) == heap_end) {
					mapping_flag = 'h';
					mem_snapshot.heap += bytes;
				} else {
					mapping_flag = 'm';
					mem_snapshot.mmap += bytes;
				}
			} else if (memcmp(owner, "[heap]", 6) == 0) {
				mapping_flag = 'h';
				mem_snapshot.heap += bytes;
			} else if (memcmp(owner, "[vvar]", 6) == 0) {
				mapping_flag = 'w';
				mem_snapshot.vvar += bytes;
			} else if (memcmp(owner, "[vdso]", 6) == 0) {
				mapping_flag = 'v';
				mem_snapshot.vdso += bytes;
			} else if (memcmp(owner, "[stack]", 7) == 0) {
				mapping_flag = 's';
				mem_snapshot.stack += bytes;
			} else if (memcmp(owner, "[vsyscall]", 10) == 0) {
				mapping_flag = 'V';
				mem_snapshot.vsyscall += bytes;
			} else {
				mapping_flag = 'u';
				mem_snapshot.unfigured += bytes;
			}
		}

		if (mapping_flag == 'h') heap_end = strtoul(addr_space_end, NULL, 16);
 		fgets_status = fgets(buffer, 1000, fin);
	}

    mem_snapshot.total_dynamic = mem_snapshot.heap + mem_snapshot.mmap;
    fclose(fin);
	return mem_snapshot;
}

#endif