/*
 * Common utility method
 */
#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

//#define DEBUG 1
#define RSIZEOF(x) sizeof(x)/sizeof(*x)

inline void DBG(const char * msg, ...);
inline char *file_get(const char *path);
inline void die(const char *perror_msg, const char * msg, ...);
int is_in(const char *needle, char *haystack[], size_t haystack_size);
void *malloc_or_die(size_t size, const char *for_name);
void *realloc_or_die(void *ptr, size_t size, const char *for_name);

#endif

