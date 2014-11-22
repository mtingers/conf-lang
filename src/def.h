#ifndef __DEF_H
#define __DEF_H

#include <stdio.h>
#include "conf-lang.h"
#include "util.h"
#include "parse.h"

struct construct_arg {
    char *arg; //[128];
};
struct construct_def {
    char *name; //[128];
    struct construct_arg *args;
    int nargs;
    int (*validate)(struct construct_def *self);
    struct data_def *data;
    int dargs;
};

enum data_def_type_t {
    UNSET,
    UNIQUE,
    MAPPED,
    OPTARG,
    ALLARG,
};

struct data_arg {
    char *arg;
};
struct data_def {
    /* 0x01 = unique and named, 0x02 = mapped to name value, 0x03 = opt 0x02*/
    char type;
    char *word;
    char **choices;
    size_t nchoices;
    //struct data_arg *args;
};

struct data_section {
    char *word;
    struct data_section *next;
    struct data_section *tail;
};

/* The main structure to get populate with user data as it comes in.
 * For example, input of "model-connection db1" results in populating
 * construct = construct_defs[0]
 * args[0][0]->arg = db1
 * argp = 0
 */
struct object {
    struct construct_def *construct; /* pointer to a construct_defs item */
    struct construct_arg **args;     /* array of construct args */
    struct data_section **data;      /* array of a list of words in data */
    int cargp;                       /* count of construct args */
    int dargp;                       /* count of data items */
    int dargi;                       /* pos of items in current data list */
    struct object *next;             /* the next object in the list */
};

void def_push(struct parse_list *pl);
void def_dump_it_all();
struct construct_def *get_construct_defs();

#endif
