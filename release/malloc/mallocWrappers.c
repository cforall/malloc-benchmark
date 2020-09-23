#ifndef malloc_Wrappers_C
#define malloc_Wrappers_C

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include <dlfcn.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mallocWrappers.h"

/*
	preloaded malloc functions
*/

int __annoying_ret_value;

void* malloc(size_t size) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[malloc called......]\n", 22);
	__malloc_preload_print_address_hex((void*)size); __annoying_ret_value = write(1, "\n", 1);
#endif
	if(__malloc_preload_status == 0) __malloc_preload_startup();
	if(__malloc_preload_status == 1) {
		__annoying_ret_value = write(1, "[using buff heap....]\n", 22);
		void* ptr =  (&__malloc_preload_buff) + __malloc_preload_buff_bumper;
		__malloc_preload_buff_bumper += size;
		assert(__malloc_preload_buff_bumper < __malloc_preload_buff_size);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[malloc ending......]\n", 22);
#endif
		return ptr;
	}

	assert(real_malloc);
	int entrance = __malloc_preload_entrance();

	size_t bookkeeping_size = entrance == 1 ? __malloc_preload_total_head_size : 0;
	void* ptr = real_malloc(size + bookkeeping_size);

	if(entrance) {
		__atomic_add_fetch(&(shmem->malloc_count), 1, __ATOMIC_SEQ_CST);
		ptr = __malloc_preload_register_allocation(ptr, bookkeeping_size, size);
	}
#ifdef MALLOC_PRELOAD_DEBUG
	else {
		__annoying_ret_value = write(1, "[malloc !! entrance]\n", 22);
	}
#endif

	__malloc_preload_exit(entrance);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[malloc ending......]\n", 22);
#endif
	return ptr;
}

void free(void *ptr) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[free called........]\n", 22);
	__malloc_preload_print_address_hex(ptr); __annoying_ret_value = write(1, "\n", 1);
#endif
	if(!ptr || __malloc_preload_status < 2) {
		real_free(ptr);
		if(__malloc_preload_status == 0) __malloc_preload_startup();
#ifdef MALLOC_PRELOAD_DEBUG
		__annoying_ret_value = write(1, "[free ending........]\n", 22);
#endif
		return;
	}
	if((char*)&__malloc_preload_buff <= (char*)ptr && (char*)ptr < ((char*)&__malloc_preload_buff + __malloc_preload_buff_size)) {
		__annoying_ret_value = write(1, "[freeing buff heap..]\n", 22);
		return;
	}

	assert(real_free);
	int entrance = __malloc_preload_entrance();

	if(entrance) {
		__atomic_add_fetch(&(shmem->free_count), 1, __ATOMIC_SEQ_CST);
		ptr = __malloc_preload_unregister_allocation(ptr);
	}

	real_free(ptr);

	__malloc_preload_exit(entrance);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[free ending........]\n", 22);
#endif
}

void *calloc(size_t nmemb, size_t size) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[calloc called......]\n", 22);
#endif
	if(__malloc_preload_status == 0) __malloc_preload_startup();
	if(__malloc_preload_status == 1) {
		void* ptr = malloc(nmemb * size);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[calloc ending......]\n", 22);
#endif
		return ptr;
	}

	assert(real_calloc);
	int entrance = __malloc_preload_entrance();

	size_t bookkeeping_size = entrance == 1 ? __malloc_preload_round_up(__malloc_preload_total_head_size, size)/size : 0;
	void* ptr = real_calloc(nmemb + bookkeeping_size, size);

	if(entrance) {
		__atomic_add_fetch(&(shmem->calloc_count), 1, __ATOMIC_SEQ_CST);
		ptr = __malloc_preload_register_allocation(ptr, bookkeeping_size*size, nmemb*size);
	}

	__malloc_preload_exit(entrance);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[calloc ending......]\n", 22);
#endif
	return ptr;
}

void *realloc(void *ptr, size_t size) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[realloc called.....]\n", 22);
#endif
	if(__malloc_preload_status == 0) __malloc_preload_startup();
	if(__malloc_preload_status == 1) {
		void* nptr = malloc(size);
		free(ptr);
		return nptr;
	}

	assert(real_realloc);
	int entrance = __malloc_preload_entrance();

	void* real_ptr = ptr && entrance == 1 ? __malloc_preload_unregister_allocation(ptr) : ptr;
	size_t bookkeeping_size = entrance == 1 ? __malloc_preload_total_head_size : 0;

/*
	// securing realloc in-case realloc done on ptr from real_malloc
	int copied_size = 0;
	void* self_copy;
	if(bookkeeping_size && real_ptr == ptr) {
		int temp__malloc_preload_func_called = __malloc_preload_func_called;
		__malloc_preload_func_called = 0;

		copied_size = real_malloc_usable_size(ptr) < size ? real_malloc_usable_size(ptr) : size;
		self_copy = malloc(copied_size);
		memcpy(self_copy, ptr, copied_size);

		__malloc_preload_func_called = temp__malloc_preload_func_called;
	}
	//
*/
	void* new_ptr = real_realloc(real_ptr, size + bookkeeping_size);

	if(entrance) {
		__atomic_add_fetch(&(shmem->realloc_count), 1, __ATOMIC_SEQ_CST);
		new_ptr = __malloc_preload_register_allocation(new_ptr, bookkeeping_size, size);
	}

/*
	// securing realloc in-case realloc done on ptr from real_malloc
	if(copied_size) {
		int temp__malloc_preload_func_called = __malloc_preload_func_called;
		__malloc_preload_func_called = 0;

		memcpy(new_ptr, self_copy, copied_size);
		free(self_copy);

		__malloc_preload_func_called = temp__malloc_preload_func_called;
	}
*/
	__malloc_preload_exit(entrance);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[realloc ending.....]\n", 22);
#endif
	return new_ptr;
}

size_t malloc_usable_size(void *ptr) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[usable called......]\n", 22);
#endif
	if(__malloc_preload_status == 0) __malloc_preload_startup();
	if(__malloc_preload_status == 1) {
#ifdef MALLOC_PRELOAD_DEBUG
		__annoying_ret_value = write(1, "[using buff heap....]\n", 22);
#endif
		real_malloc_usable_size = dlsym(RTLD_NEXT, "malloc_usable_size");
		dlerror();	assert(real_malloc_usable_size);
		return real_malloc_usable_size(ptr);
	}

	if(memcmp((char*)ptr - __malloc_preload_canary_size, __malloc_preload_canary, __malloc_preload_canary_size) != 0) {
#ifdef MALLOC_PRELOAD_DEBUG
		__annoying_ret_value = write(1, "[CANARY MISMATCHED..]\n", 22);
#endif
		size_t usable_size = real_malloc_usable_size ? real_malloc_usable_size(ptr) : 0;
		return usable_size;
	}

	size_t* real_ptr = (size_t*)((uintptr_t)ptr - __malloc_preload_total_head_size);
	size_t requested_size = real_ptr[0];
	size_t usable_size = real_malloc_usable_size ? real_malloc_usable_size(real_ptr) : 0;
	assert((!usable_size && requested_size > 0) || (requested_size <= usable_size));
	if(!usable_size && (requested_size > 0)) {
		usable_size = requested_size;
		__annoying_ret_value = write(1, "[USABLE SIZE !FOUND.]\n", 22);
		__atomic_add_fetch(&(shmem->reallocarray_count), 1, __ATOMIC_SEQ_CST);
	}
	assert(requested_size <= usable_size);
	usable_size -= __malloc_preload_total_head_size;

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[usable ending......]\n", 22);
#endif
	return usable_size;
}

/*
	dynamic memory stats
*/
int __malloc_preload_entrance(void) {
	assert(__malloc_preload_func_called == 0 || __malloc_preload_func_called == 1);
	int entrance = __malloc_preload_func_called == 0 ? 1 : 0;
	__atomic_add_fetch(&(__malloc_preload_func_called), entrance, __ATOMIC_SEQ_CST);
	assert(__malloc_preload_func_called == 0 || __malloc_preload_func_called == 1); // cautionary sanity check
	return entrance;
}

void __malloc_preload_exit(int entrance) {
	assert(__malloc_preload_func_called == 0 || __malloc_preload_func_called == 1);
	__atomic_sub_fetch(&(__malloc_preload_func_called), entrance, __ATOMIC_SEQ_CST);
	assert(__malloc_preload_func_called == 0 || __malloc_preload_func_called == 1); // cautionary sanity check
}

void* __malloc_preload_register_allocation(void* real_ptr, size_t bookkeeping_size, size_t size) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[registered ptr.....]\n", 22);
	__malloc_preload_print_address_hex(real_ptr); __annoying_ret_value = write(1, "\n", 1);
#endif

	assert(__malloc_preload_total_head_size <= bookkeeping_size);
	size_t requested_size = bookkeeping_size + size;
	size_t usable_size = real_malloc_usable_size ? real_malloc_usable_size(real_ptr) : 0;
	assert((!usable_size && requested_size > 0) || (requested_size <= usable_size));
	if(!usable_size && (requested_size > 0)) {
		usable_size = requested_size;
		__annoying_ret_value = write(1, "[USABLE SIZE !FOUND.]\n", 22);
	}
	assert(requested_size <= usable_size);

	size_t* ptr = (size_t*)real_ptr;
	ptr[0] = requested_size;
	memcpy((char*)ptr + __malloc_preload_head_size, __malloc_preload_canary, __malloc_preload_canary_size);
	ptr = (size_t*)((uintptr_t)ptr + __malloc_preload_total_head_size);

	__atomic_add_fetch(&(shmem->requested_memory), requested_size, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&(shmem->usable_allocation), usable_size, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&(shmem->current_requested_memory), requested_size, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&(shmem->current_usable_allocation), usable_size, __ATOMIC_SEQ_CST);

#ifdef MALLOC_PRELOAD_DEBUG
	__malloc_preload_print_address_hex((void*)ptr); __annoying_ret_value = write(1, "\n", 1);
#endif
	assert(shmem->current_usable_allocation >= shmem->current_requested_memory);
	return (void*)ptr;
}

void* __malloc_preload_unregister_allocation(void* ptr) {
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[unregistered ptr...]\n", 22);
	__malloc_preload_print_address_hex((void*)ptr); __annoying_ret_value = write(1, "\n", 1);
#endif

	if(memcmp((char*)ptr - __malloc_preload_canary_size, __malloc_preload_canary, __malloc_preload_canary_size) != 0) {
#ifdef MALLOC_PRELOAD_DEBUG
		__annoying_ret_value = write(1, "[CANARY MISMATCHED..]\n", 22);
#endif
		return ptr;
	}

	size_t* real_ptr = (size_t*)((uintptr_t)ptr - __malloc_preload_total_head_size);
	size_t requested_size = real_ptr[0];
	size_t usable_size = real_malloc_usable_size ? real_malloc_usable_size(real_ptr) : 0;
	assert((!usable_size && requested_size > 0) || (requested_size <= usable_size));
	if(!usable_size && (requested_size > 0)) {
		usable_size = requested_size;
		__annoying_ret_value = write(1, "[USABLE SIZE !FOUND.]\n", 22);
	}
	assert(requested_size <= usable_size);

	__atomic_sub_fetch(&(shmem->current_requested_memory), requested_size, __ATOMIC_SEQ_CST);
	__atomic_sub_fetch(&(shmem->current_usable_allocation), usable_size, __ATOMIC_SEQ_CST);

#ifdef MALLOC_PRELOAD_DEBUG
	__malloc_preload_print_address_hex(real_ptr); __annoying_ret_value = write(1, "\n", 1);
#endif
	assert(shmem->current_usable_allocation >= shmem->current_requested_memory);
	return (void*)real_ptr;
}

/*
	preload constructor/destructor/helpers
*/
void __malloc_preload_startup (void) {
	__annoying_ret_value = write(1, "[malloc preloading..]\n", 22);
	if(__malloc_preload_status == 0) {
		__atomic_add_fetch(&__malloc_preload_status, 1, __ATOMIC_SEQ_CST);
		__malloc_preload_initialize();
		__atomic_add_fetch(&__malloc_preload_status, 1, __ATOMIC_SEQ_CST);
		__annoying_ret_value = write(1, "[preloading done....]\n", 22);
	}
	__annoying_ret_value = write(1, "[malloc preloaded...]\n", 22);
}

void __malloc_preload_sanity_check(void) {
	__annoying_ret_value = write(1, "[PRELOAD SANE???????]\n", 22);
	assert(__malloc_preload_status == 2);

	size_t size = 57 * sizeof(char);

	/*
		malloc check
	*/
	char* malloc_ptr = malloc(size);
	assert(malloc_ptr);
	memcpy(malloc_ptr, "DEADBEEF\0", 9);

	size_t usable_size = malloc_usable_size(malloc_ptr);
	assert(size <= usable_size);

	void* malloc_real_ptr = __malloc_preload_unregister_allocation(malloc_ptr);

	size_t bookkeeping_size = (uintptr_t)malloc_ptr - (uintptr_t)malloc_real_ptr;
	assert(bookkeeping_size == 0 || bookkeeping_size == __malloc_preload_total_head_size);
	if(bookkeeping_size == 0)
		__annoying_ret_value = write(1, "[SEE CANARY MSG ABV?]\n", 22);
	else
		assert(malloc_ptr == __malloc_preload_register_allocation(malloc_real_ptr, bookkeeping_size, size));

	assert(memcmp(malloc_ptr, "DEADBEEF\0", 9) == 0);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[MALLOC........CHECK]\n", 22);
#endif

	/*
		realloc check
	*/
	size = size * 2;
	malloc_ptr = realloc(malloc_ptr, size);
	assert(malloc_ptr);
	assert(memcmp(malloc_ptr, "DEADBEEF\0", 9) == 0);

	usable_size = malloc_usable_size(malloc_ptr);
	assert(size <= usable_size);

	malloc_real_ptr = __malloc_preload_unregister_allocation(malloc_ptr);

	bookkeeping_size = (uintptr_t)malloc_ptr - (uintptr_t)malloc_real_ptr;
	assert(bookkeeping_size == 0 || bookkeeping_size == __malloc_preload_total_head_size);
	if(bookkeeping_size == 0)
		__annoying_ret_value = write(1, "[SEE CANARY MSG ABV?]\n", 22);
	else
		assert(malloc_ptr == __malloc_preload_register_allocation(malloc_real_ptr, bookkeeping_size, size));

	assert(memcmp(malloc_ptr, "DEADBEEF\0", 9) == 0);

	free(malloc_ptr); malloc_ptr = NULL;

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[REALLOC.......CHECK]\n", 22);
#endif

	/*
		calloc check
	*/
	size_t nmemb = 1;
	malloc_ptr = calloc(nmemb, size);
	assert(malloc_ptr);
	memcpy(malloc_ptr, "DEADBEEF\0", 9);

	usable_size = malloc_usable_size(malloc_ptr);
	assert(size * nmemb <= usable_size);

	malloc_real_ptr = __malloc_preload_unregister_allocation(malloc_ptr);

	bookkeeping_size = (uintptr_t)malloc_ptr - (uintptr_t)malloc_real_ptr;
	assert(bookkeeping_size == 0 || bookkeeping_size == __malloc_preload_total_head_size);
	if(bookkeeping_size == 0)
		__annoying_ret_value = write(1, "[SEE CANARY MSG ABV?]\n", 22);
	else
		assert(malloc_ptr == __malloc_preload_register_allocation(malloc_real_ptr, bookkeeping_size, size));

	assert(memcmp(malloc_ptr, "DEADBEEF\0", 9) == 0);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[CALLOC........CHECK]\n", 22);
#endif

	free(malloc_ptr); malloc_ptr = NULL;
	__annoying_ret_value = write(1, "[YES!!! PRELOAD SANE]\n", 22);
    __annoying_ret_value = write(1, "\n", 1);
}

void __malloc_preload_cleanup (void) {
	//fflush(stdout);
    __annoying_ret_value = write(1, "\n", 1);
	__annoying_ret_value = write(1, "[malloc preload end.]\n", 22);
	__malloc_preload_print_stats(); printf("\n");
	//fflush(stdout);
	end_ipc();
}

void __malloc_preload_print_stats(void) {
	print_int_malloc_stats_head(NULL); printf("\n");
	print_int_malloc_stats(*shmem, NULL);
}

char __malloc_preload_hex_digit(int v) {
    if (v >= 0 && v < 10)
        return '0' + v;
    else
        return 'a' + v - 10;
}

void __malloc_preload_print_address_hex(void* p0) {
    int i;
    uintptr_t p = (uintptr_t)p0;

    __annoying_ret_value = write(1, "[0x", 3);
    for(i = (sizeof(p) << 3) - 4; i>=0; i -= 4) {
        __malloc_preload_writechar(__malloc_preload_hex_digit((p >> i) & 0xf));
    }
    __annoying_ret_value = write(1, "]", 1);
}

void __malloc_preload_writechar(char c) {
	__annoying_ret_value = write(1, &c, 1);
}

size_t __malloc_preload_round_up(size_t n, size_t divider) {
	size_t result = (n % divider) == 0 ? n : n + divider - (n % divider);
	return result;
}

void __malloc_preload_initialize(void) {
#ifdef MALLOC_PRELOAD_DEBUG
	__malloc_preload_print_address_hex((void*)__malloc_preload_head_size); __annoying_ret_value = write(1, " :head..\n", 9);
	__malloc_preload_print_address_hex((void*)__malloc_preload_canary_size); __annoying_ret_value = write(1, " :canary\n", 9);
	__malloc_preload_print_address_hex((void*)__malloc_preload_total_head_size); __annoying_ret_value = write(1, " :total.\n", 9);
    /*
    	malloc overloading
    */
	__annoying_ret_value = write(1, "[malloc doing.......]\n", 22);
#endif

	real_malloc = dlsym(RTLD_NEXT, "malloc");
	dlerror(); assert(real_malloc);
	void* (*def_malloc)(size_t) = dlsym(RTLD_DEFAULT, "malloc");
    dlerror(); assert(def_malloc);
	assert(def_malloc != real_malloc);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[malloc done........]\n", 22);
	__malloc_preload_print_address_hex(def_malloc); __annoying_ret_value = write(1, "\n", 1);
	__malloc_preload_print_address_hex(real_malloc); __annoying_ret_value = write(1, "\n", 1);

    /*
    	free overloading
    */
	__annoying_ret_value = write(1, "[free doing.........]\n", 22);
#endif

	real_free = dlsym(RTLD_NEXT, "free");
	dlerror(); assert(real_free);
	void (*def_free)(void*) = dlsym(RTLD_DEFAULT, "free");
    dlerror(); assert(def_free);
	assert(def_free != real_free);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[free done..........]\n", 22);
	__malloc_preload_print_address_hex(def_free); __annoying_ret_value = write(1, "\n", 1);
	__malloc_preload_print_address_hex(real_free); __annoying_ret_value = write(1, "\n", 1);

    /*
    	calloc overloading
    */
	__annoying_ret_value = write(1, "[calloc doing.......]\n", 22);
#endif

	real_calloc = dlsym(RTLD_NEXT, "calloc");
	dlerror(); assert(real_calloc);
	void* (*def_calloc)(size_t, size_t) = dlsym(RTLD_DEFAULT, "calloc");
    dlerror(); assert(def_calloc);
	assert(def_calloc != real_calloc);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[calloc done........]\n", 22);
	__malloc_preload_print_address_hex(def_calloc); __annoying_ret_value = write(1, "\n", 1);
	__malloc_preload_print_address_hex(real_calloc); __annoying_ret_value = write(1, "\n", 1);

    /*
    	realloc overloading
    */
	__annoying_ret_value = write(1, "[realloc doing......]\n", 22);
#endif

	real_realloc = dlsym(RTLD_NEXT, "realloc");
	dlerror(); assert(real_realloc);
	void* (*def_realloc)(void*, size_t) = dlsym(RTLD_DEFAULT, "realloc");
    dlerror(); assert(def_realloc);
	assert(def_realloc != real_realloc);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[realloc done.......]\n", 22);
	__malloc_preload_print_address_hex(def_realloc); __annoying_ret_value = write(1, "\n", 1);
	__malloc_preload_print_address_hex(real_realloc); __annoying_ret_value = write(1, "\n", 1);

    /*
    	malloc_usable_size overloading
    */
	__annoying_ret_value = write(1, "[usable doing.......]\n", 22);
#endif

	real_malloc_usable_size = dlsym(RTLD_NEXT, "malloc_usable_size");
	dlerror(); assert(real_malloc_usable_size);
	size_t (*def_malloc_usable_size)(void*) = dlsym(RTLD_DEFAULT, "malloc_usable_size");
    dlerror(); assert(def_malloc_usable_size);
	assert(def_malloc_usable_size != real_malloc_usable_size);

#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[usable done........]\n", 22);
	__malloc_preload_print_address_hex(def_malloc_usable_size); __annoying_ret_value = write(1, "\n", 1);
	__malloc_preload_print_address_hex(real_malloc_usable_size); __annoying_ret_value = write(1, "\n", 1);
#endif

    /*
    	initiate stat counters
    */
	init_ipc();
	assert(shmem);
#ifdef MALLOC_PRELOAD_DEBUG
	__annoying_ret_value = write(1, "[shared memory done.]\n", 22);
#endif

//	shmem->malloc_count = 0;
//	shmem->free_count = 0;
//	shmem->calloc_count = 0;
//	shmem->realloc_count = 0;
//	shmem->reallocarray_count = 0;

//	shmem->requested_memory = 0;
//	shmem->usable_allocation = 0;
//	shmem->current_requested_memory = 0;
//	shmem->current_usable_allocation = 0;

	shmem->real_malloc = real_malloc;
	shmem->real_free = real_free;
	shmem->real_calloc = real_calloc;
	shmem->real_realloc = real_realloc;
	shmem->real_malloc_usable_size = real_malloc_usable_size;
}

#endif // malloc_Wrappers_C
