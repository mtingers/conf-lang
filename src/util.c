
#include "util.h"
#include <stdarg.h>

inline void DBG(const char * msg, ...)
{
#ifdef DEBUG
    va_list arg;
    fprintf(stdout, "DEBUG:");
    va_start(arg, msg);
    vfprintf(stdout, msg, arg);
    va_end(arg);
    fprintf(stdout, "\n");
#endif
}

inline void die(const char *perror_msg, const char * msg, ...)
{
    va_list arg;
    if(perror_msg)
        perror(perror_msg);
    va_start(arg, msg);
    vfprintf(stderr, msg, arg);
    va_end(arg);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

inline char *file_get(const char *path)
{
    char *p = NULL;
    FILE *f = NULL;
    struct stat st;
    int rc = lstat(path, &st);

    if(rc != 0)
        goto error;

    f = fopen(path, "r");
    if(!f)
        goto error;

    /* while( (p = fgets(b, PARSE_BUF_SIZE, f)) ) {} */
    /* read the whole damn thing into memory with a magic limit */
    if((p = malloc(sizeof(*p)*st.st_size+1)) == NULL)
        goto error;

    if(fread(p, st.st_size, 1, f) != 1)
        goto error;
    fclose(f);
    p[st.st_size] = '\0'; /* just in case */
    return p;

error:
    if(p)
        free(p);
    if(f)
        fclose(f);
    return NULL;
}

int is_in(const char *needle,  char *haystack[], size_t haystack_size)
{
    size_t i = 0;
    char **p = haystack;
    for(; i < haystack_size; i++)
        if(strcmp(p[i], needle) == 0)
            return i;
    return 0;
}

void *malloc_or_die(size_t size, const char *for_name)
{
    void *p = malloc(size);
    if(!p) {
        if(for_name)
            die("malloc", "ERROR: Failed to allocate %zd bytes for %s",
                size, for_name);
        else
            die("malloc", "ERROR: Failed to allocate %zd bytes", size);
    }
    return p;
}

void *realloc_or_die(void *ptr, size_t size, const char *for_name)
{
    void *p = realloc(ptr, size);
    if(!p) {
        if(for_name)
            die("realloc", "ERROR: Failed to allocate %zd bytes for %s",
                size, for_name);
        else
            die("realloc", "ERROR: Failed to allocate %zd bytes", size);
    }
    return p;
}
