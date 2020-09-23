#ifndef data_Structures_H
#define data_Structures_H

#include <stdio.h>
#include <stdlib.h>

int verbose_flag;

typedef struct S_int_malloc_statistics {
	int ppid;
	int pid;

	unsigned long int malloc_count;
	unsigned long int free_count;
	unsigned long int calloc_count;
	unsigned long int realloc_count;
	unsigned long int reallocarray_count;

	unsigned long long requested_memory;
	unsigned long long usable_allocation;
	unsigned long long current_requested_memory;
	unsigned long long current_usable_allocation;

	// will be set by preload and used in speed benchmark
	void* (*real_malloc)(size_t);
	void* (*real_calloc)(size_t, size_t);
	void* (*real_realloc)(void*, size_t);
	void (*real_free)(void*);
	size_t (*real_malloc_usable_size)(void*);

	// will be used by threads in speed benchmark
	int thread_n;
	char thread_chat[11];
} T_int_malloc_stats; // S_internal malloc statistics
void print_int_malloc_stats_head(FILE* fout);
void print_int_malloc_stats(T_int_malloc_stats obj, FILE* fout);
T_int_malloc_stats parse_int_malloc_stats(FILE* fin);

typedef struct S_fragmentation {
	long double alignment;
	long double bookkeeping; // NOT YET

	long double internal; // alignment + bookkeeping
	long double external;

	long double total;
	long long int ref_bytes;
} T_fragmentation; // S_fragmentation
void print_fragmentation_head(FILE* fout);
void print_fragmentation(T_fragmentation obj, FILE* fout);

typedef struct S_memory_snapshot {
	T_int_malloc_stats int_malloc_stats;

	long int text;

	unsigned long int heap;
	unsigned long int mmap_so;
	unsigned long int mmap;

	unsigned long int stack;
	unsigned long int vvar;
	unsigned long int vdso;
	unsigned long int vsyscall;

	unsigned long int unfigured;
	unsigned long int total_dynamic;

	unsigned long long timestamp_ms;

	T_fragmentation fragmentation;
} T_memory_snapshot; // S_memory snapshot
void print_memory_snapshot_head(FILE* fout);
void print_memory_snapshot_detail_head(FILE* fout);
void print_memory_snapshot(T_memory_snapshot obj, FILE* fout);
void print_memory_snapshot_detail(T_memory_snapshot obj, FILE* fout);
T_memory_snapshot parse_memory_snapshot(FILE* fin);

#endif // data_Structures_H