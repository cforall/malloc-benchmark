#ifndef data_Structures_C
#define data_Structures_C

#include <string.h>

#include "dataStructures.h"

void print_int_malloc_stats_head(FILE* fout){
	if(!fout) fout = stdout;

	fprintf(fout, "\
  ppid \
   pid \
        malloc_count \
          free_count \
        calloc_count \
       realloc_count \
 realloc_array_count \
     requsted_mem(B) \
       usable_mem(B) \
  current_req_mem(B) \
  current_use_mem(B)");
}

void print_int_malloc_stats(T_int_malloc_stats obj, FILE* fout){
	if(!fout) fout = stdout;

	fprintf(fout, "%6d %6d %20lu %20lu %20lu %20lu %20lu %20llu %20llu %20llu %20llu",
		obj.ppid,
		obj.pid,
		obj.malloc_count,
		obj.free_count,
		obj.calloc_count,
		obj.realloc_count,
		obj.reallocarray_count,
		obj.requested_memory,
		obj.usable_allocation,
		obj.current_requested_memory,
		obj.current_usable_allocation);
}

T_int_malloc_stats parse_int_malloc_stats(FILE* fin) {
	T_int_malloc_stats int_malloc_stats = {0};
	char buffer[1000];
	char* fgets_status = fgets(buffer, 999, fin);
	if(!fgets_status) {
		int_malloc_stats.ppid = -1; // flag end of file
		return int_malloc_stats;
	}
	char* token = (char*) strtok(buffer, " ");
	int_malloc_stats.ppid = (int)strtol(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.pid = (int)strtol(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.malloc_count = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.free_count = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.calloc_count = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.realloc_count = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.reallocarray_count = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.requested_memory = strtoull(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.usable_allocation = strtoull(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.current_requested_memory = strtoull(token, NULL, 10);
	token = strtok(NULL, " ");
	int_malloc_stats.current_usable_allocation = strtoull(token, NULL, 10);
	if(int_malloc_stats.current_requested_memory > int_malloc_stats.current_usable_allocation) {
		printf("WARN: parsing current_requested_memory > current_usable_allocation\n");
	}
	return int_malloc_stats;
}

void print_fragmentation_head(FILE* fout){
	if(!fout) fout = stdout;

	fprintf(fout, "\
             alignment(xReq) \
           bookkeeping(xReq) \
              int_frag(xReq) \
              ext_frag(xReq) \
                 total(xReq) \
                   Req(B)");
}

void print_fragmentation(T_fragmentation obj, FILE* fout){
	if(!fout) fout = stdout;

	fprintf(fout, "%25.2Lf %25.2Lf %25.2Lf %25.2Lf %25.2Lf %25lld",
		obj.alignment,
		obj.bookkeeping,
		obj.internal,
		obj.external,
		obj.total,
		obj.ref_bytes);
}

void print_memory_snapshot_head(FILE* fout){
	if(!fout) fout = stdout;

	fprintf(fout, "\
                text \
                heap \
             mmap_so \
                mmap \
               stack \
                vvar \
                vdso \
            vsyscall \
           unfigured \
       total_dynamic \
 epoch_timestamp(ms)");
}

void print_memory_snapshot(T_memory_snapshot obj, FILE* fout){
	if(!fout) fout = stdout;

	fprintf(fout, "%20lu %20lu %20lu %20lu %20lu %20lu %20lu %20lu %20lu %20lu %20llu",
		obj.text,
		obj.heap,
		obj.mmap_so,
		obj.mmap,
		obj.stack,
		obj.vvar,
		obj.vdso,
		obj.vsyscall,
		obj.unfigured,
		obj.total_dynamic,
		obj.timestamp_ms);
}

void print_memory_snapshot_detail_head(FILE* fout){
	if(!fout) fout = stdout;

	print_int_malloc_stats_head(fout);
	fprintf(fout, " ");
	print_memory_snapshot_head(fout);
	fprintf(fout, " ");
	print_fragmentation_head(fout);
	fprintf(fout, "\n");
}

void print_memory_snapshot_detail(T_memory_snapshot obj, FILE* fout) {
	if(!fout) fout = stdout;

	print_int_malloc_stats(obj.int_malloc_stats, fout);
	fprintf(fout, " ");
	print_memory_snapshot(obj, fout);
	fprintf(fout, " ");
	print_fragmentation(obj.fragmentation, fout);
	fprintf(fout, "\n");
}

T_memory_snapshot parse_memory_snapshot(FILE* fin){
	T_memory_snapshot mem_snapshot = {0};
	char buffer[1000];
	char* fgets_status = fgets(buffer, 999, fin);
	if(!fgets_status) {
		mem_snapshot.text = -1; // flag end of file
		return mem_snapshot;
	}
	char* token = (char*) strtok(buffer, " ");
	mem_snapshot.text = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.heap = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.mmap_so = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.mmap = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.stack = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.vvar = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.vdso = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.vsyscall = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.unfigured = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.total_dynamic = strtoul(token, NULL, 10);
	token = strtok(NULL, " ");
	mem_snapshot.timestamp_ms = strtoull(token, NULL, 10);
	if(mem_snapshot.total_dynamic != mem_snapshot.heap + mem_snapshot.mmap + mem_snapshot.mmap_so) {
    	printf("WARN: benchmark: total_dynamic != heap + mmap + mmap_so\n");
    }
    return mem_snapshot;
}

#endif // data_Structures_C