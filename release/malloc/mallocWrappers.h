#ifndef malloc_Wrappers_H
#define malloc_Wrappers_H

#include "stdint.h"
#include <malloc.h>
#include <math.h>
#include <threads.h> // thread_local

#include "utilities.h"

/*
	temporary malloc heap
*/
#ifndef malloc_Proxy_Naive_C
size_t __malloc_preload_buff_bumper = 0; // static
size_t __malloc_preload_buff_size = 16384; // static
char __malloc_preload_buff[16384]; // static

/*
	preload constructor/destructor/helpers
*/
int __malloc_preload_status = 0; // static
#endif
void __malloc_preload_startup (void) __attribute__ ((constructor (111)));
void __malloc_preload_sanity_check (void) __attribute__ ((constructor (112)));
void __malloc_preload_cleanup (void) __attribute__ ((destructor (111)));
char __malloc_preload_hex_digit(int v);
void __malloc_preload_print_address_hex(void* p0);
void __malloc_preload_writechar(char c);
void __malloc_preload_initialize(void);
void __malloc_preload_print_stats(void);
size_t __malloc_preload_round_up(size_t n, size_t divider);

/*
	real malloc functions
*/
void* (*real_malloc)(size_t) = NULL; // static
void* (*real_calloc)(size_t, size_t) = NULL; // static
void* (*real_realloc)(void*, size_t) = NULL; // static
void (*real_free)(void*) = NULL; // static
size_t (*real_malloc_usable_size)(void*) = NULL; // static

/*
	preloaded malloc functions
*/
void* malloc(size_t size);
void free(void *ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void *ptr, size_t size);
size_t malloc_usable_size(void *ptr);

/*
	dynamic memory stats
*/
thread_local int __malloc_preload_func_called = 0;
const char __malloc_preload_canary[] = "MPrLDBf";
const size_t __malloc_preload_head_size = sizeof(size_t);
const size_t __malloc_preload_canary_size = sizeof(__malloc_preload_canary);
const size_t __malloc_preload_total_head_size = (__malloc_preload_head_size + __malloc_preload_canary_size) % 16 == 0 ?
												__malloc_preload_head_size + __malloc_preload_canary_size :
												__malloc_preload_head_size + __malloc_preload_canary_size + 16 -
												((__malloc_preload_head_size + __malloc_preload_canary_size) % 16);
int __malloc_preload_entrance(void);
void __malloc_preload_exit(int entrance);
void* __malloc_preload_register_allocation(void* real_ptr, size_t bookkeeping_size, size_t size);
void* __malloc_preload_unregister_allocation(void* ptr);

#endif // malloc_Wrappers_H
