#ifndef utilities_H
#define utilities_H

#include "dataStructures.h"

#ifndef SHM_KEY
#define SHM_KEY 0x123458
#endif

int shmid;
T_int_malloc_stats* shmem;

/*
	init_ipc attaches(creates it if it is not already created) to a shared memory region by SHM_KEY for inter-process communication
	ipc is used to share malloc stats between malloc-wrappers and benchmark program
	input: NONE
	output: NONE
	effects: may change value of T_int_malloc_stats* shmem or print error message(s) to stdout
*/
void init_ipc();

/*
	set_ipc access global variable shmem and sets every variable in it to zero, does nothing if shmeme is NULL
	input: NONE
	output: NONE
	effects: may change values inside T_int_malloc_stats* shmem
*/
//void  set_ipc();

/*
	end_ipc detaches ptocess from shared memory region pointed by T_int_malloc_stats* shmem and marks it for destruction
	input: NONE
	output: NONE
	effects: may print error message(s) to stdout
*/
void end_ipc();

unsigned long long get_cur_time_ms(int print);

/*
	parse_proc_maps reads maps file of process by given pid as proc/pid/maps and populates fields of T_memory_snapshot defined in common/dataStructures.h
	input: an int (pid of process whose maps file is to be read and parsed, -1 for self)
	output: a variable of type T_memory_snapshot
	effects: may print error message(s)
*/
T_memory_snapshot parse_proc_maps(const int pid);

#endif