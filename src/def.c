#include "def.h"
#include "util.h"

/*****************************************************/
/*  model-connection <model-connection-name> {
        name <arg>
        pass <arg>
        user <arg>
        host <arg>
        type <arg>
    }
*/
static char *model_connection_type_choices[] = {
    NULL,
    "mysql",
    "postgresql",
};
static struct data_def model_connection_data_defs[] = {
    {UNIQUE, "name", NULL, 0},
    {UNIQUE, "user", NULL, 0},
    {UNIQUE, "pass", NULL, 0},
    {UNIQUE, "host", NULL, 0},
    {UNIQUE, "type", model_connection_type_choices,
        RSIZEOF(model_connection_type_choices)},
};
static struct construct_arg model_connection_args[] = {
    {"name"},
};
/*****************************************************/
/*****************************************************/
/*  model <model-connection-name> <model-name> {
        <field> <type> <type-arg...>
        ...
    }
*/
static char *model_field_choices[] = {
    NULL,
    "int",
    "bigint",
    "bool",
    "char",
    "date",
    "datetime",
    "decimal",
    "float",
    "text",
    "foreignkey",
};
/* if ends in '=', use strncmp to match partial */
static char *model_field_optional_choices[] = {
    NULL,
    "null",
    "notnull",
    "default=",
    "primarykey",
    "unique",
};
static struct data_def model_data_defs[] = {
    {MAPPED, "name", NULL, 0},
    {MAPPED, "type", model_field_choices,
        RSIZEOF(model_field_choices)},
    {OPTARG, "opt1", model_field_optional_choices,
        RSIZEOF(model_field_optional_choices)},
    {OPTARG, "opt2", model_field_optional_choices,
        RSIZEOF(model_field_optional_choices)},
    {OPTARG, "opt3", model_field_optional_choices,
        RSIZEOF(model_field_optional_choices)},
    {OPTARG, "opt4", model_field_optional_choices,
        RSIZEOF(model_field_optional_choices)},
};
static struct construct_arg model_args[] = {
    {"model_connection_name"}, {"name"},
};
/*****************************************************/
/*****************************************************/
/*  url {
        <url>
        ...
    }
*/
static struct data_def url_data_defs[] = {
    {ALLARG, "url", NULL, 0},
};
//static struct construct_arg url_args[] = {};
/*****************************************************/
/*****************************************************/
/*  action <action-name> {
        <url> <whence> <resource> <resource-arg>
        ...
    }
*/
static char *action_data_whence_choices[] = {NULL, "all", "post", "get"};
static char *action_data_resource_choices[] = {NULL, "http", "https", "exec"};

static struct data_def action_data_defs[] = {
    {MAPPED, "url", NULL, 0},
    {MAPPED, "whence", action_data_whence_choices,
        RSIZEOF(action_data_whence_choices)},
    {MAPPED, "resource", action_data_resource_choices,
        RSIZEOF(action_data_resource_choices)},
    {MAPPED, "resource-arg", NULL, 0},
};
static struct construct_arg action_args[] = {{"name"}};

/*****************************************************/
/*****************************************************/
/*  auth-url {
        <url>
        ...
    }
*/
static struct data_def auth_url_data_defs[] = {
    {ALLARG, "url", NULL, 0},
};
/*****************************************************/
/*****************************************************/
/*  auth-url-restriction {
        <url> <model-connection> <model>
        ...
    }
*/
static struct data_def auth_url_restriction_data_defs[] = {
    {MAPPED, "url", NULL, 0},
    {MAPPED, "model-connection", NULL, 0},
    {MAPPED, "model", NULL, 0},
};
/*****************************************************/
/*****************************************************/
/*  static-path {
        <directory>
        ...
    }
*/
static struct data_def static_data_defs[] = {
    {ALLARG, "directory", NULL, 0},
};
/*****************************************************/
/*****************************************************/
/*  template-path {
        <directory>
        ...
    }
*/
static struct data_def template_data_defs[] = {
    {ALLARG, "directory", NULL, 0},
};
/*****************************************************/
/*****************************************************/
/*  template-var <name> {
        <value>
    }
*/
static struct data_def template_var_data_defs[] = {
    {ALLARG, "value", NULL, 0},
};
static struct construct_arg template_var_args[] = {{"var_name"}};
/*****************************************************/


/*****************************************************/
/* The primary structure that names and references all other constructs */
static struct construct_def construct_defs[] = {
    {
        "model-connection",
        model_connection_args, RSIZEOF(model_connection_args),
        NULL,
        model_connection_data_defs, RSIZEOF(model_connection_data_defs)
    },{
        "model",
        model_args, RSIZEOF(model_args),
        NULL,
        model_data_defs, RSIZEOF(model_data_defs)
    },{
        "auth-url",
        NULL, 0,
        NULL,
        auth_url_data_defs, RSIZEOF(auth_url_data_defs)
    },{
        "auth-url-restriction",
        NULL, 0,
        NULL,
        auth_url_restriction_data_defs, RSIZEOF(auth_url_restriction_data_defs)
    },{
        "url",
        NULL, 0,
        NULL,
        url_data_defs, RSIZEOF(url_data_defs)
    },{
        "template-path",
        NULL, 0,
        NULL,
        template_data_defs, RSIZEOF(template_data_defs)
    },{
        "static-path",
        NULL, 0,
        NULL,
        static_data_defs, RSIZEOF(static_data_defs)
    },{
        "template-var",
        template_var_args, RSIZEOF(template_var_args),
        NULL,
        template_var_data_defs, RSIZEOF(template_var_data_defs)
    },{
        "action",
        action_args, RSIZEOF(action_args),
        NULL,
        action_data_defs, RSIZEOF(action_data_defs)
    },
};

struct object *objects = NULL, *objects_tail = NULL;

struct construct_def *get_construct_defs()
{
    return construct_defs;
}

static struct object *def_new_object()
{
    struct object *o = malloc(sizeof(*o));
    if(!o)
        die("malloc", "ERROR: Failed to allocate objects structure");
    o->construct = NULL;
    o->args = NULL;
    o->cargp = 0;
    o->dargp = 0;
    o->data = NULL;
    o->next = NULL;
    return o;
}

static void def_print_construct(struct construct_def *d)
{
#ifdef DEBUG
    int i = 0;
    printf("construct_def: %s\n", d->name);
    for(; i < d->nargs; i++)
        printf("\targ[%d]=%s\n", i, d->args[i].arg);
#endif
}

static void def_insert_new_object(struct parse_list *pl)
{
    size_t i = 0, x = 0;
    struct object *cur = NULL;
    DBG("def_insert_new_object");
    if(!objects) {
        objects = def_new_object();
        cur = objects;
        objects_tail = objects;
    } else {
        cur = def_new_object();
        objects_tail->next = cur;
        objects_tail = cur;
    }

    /* find the construct name */
    for(; i < RSIZEOF(construct_defs); i++) {
        if(strcmp(pl->word, construct_defs[i].name) == 0) {
            cur->construct = &construct_defs[i];
            /* preallocate here the expect args */
            cur->args = malloc_or_die(sizeof(*cur->args)*cur->construct->nargs,
                "construct object args");
            for(x = 0; x < cur->construct->nargs; x++)
                cur->args[x] = malloc_or_die(sizeof(*cur->args[x]),
                    "construct object arg");
            def_print_construct(cur->construct);
            return;
        }
    }
    die(NULL, "ERROR: Could not find construct definition `%s' on line %zd",
        pl->word, pl->line_number);
}

/* make sure all args in a construct are a unique set */
/*static void def_validate_word_constraints(struct parse_list *pl)
{
    //struct object *o = objects;

}*/

static void def_insert_construct_arg(struct parse_list *pl)
{
    struct object *o = objects_tail;
    //def_validate_word_constraints(pl);
    if(o->cargp >= o->construct->nargs)
        die(NULL, "ERROR: Too many arguments to %s construct on line %zd "
            "near word `%s'",
            o->construct->name, pl->line_number, pl->word);
    o->args[o->cargp]->arg = pl->word;
    DBG("def_insert_construct_arg: o->args[%d]->arg=%s",
        o->cargp, o->args[o->cargp]->arg);
    o->cargp++;

    /* this should be the last argument. let us validate the uniqueness */
    /* TODO: Optimize this as it can be terribly slow with craptons of tables
     * (unrealistic edge-case) */
    if(o->cargp >= o->construct->nargs) {
        struct object *cur = objects;
        size_t x, equal = 0;
        while(cur) {
            equal = 0;
            // same construct then check all args
            if(strcmp(cur->construct->name, o->construct->name) == 0 &&
                                                                    o != cur) {
                for(x = 0; x < cur->cargp; x++)
                    if(strcmp(cur->args[x]->arg, o->args[x]->arg) == 0)
                        equal++;
                if(equal >= cur->cargp) {
                    DBG("equal >= cur->cargp : %zd >= %zd", equal, cur->cargp);
                    die(NULL,
                        "ERROR: %s already defined identifier `%s' line %zd",
                        o->construct->name, pl->word, pl->line_number);
                }
            }
            cur = cur->next;
        }
    }
}

static void def_validate_choices(struct object *o, struct parse_list *pl)
{
    size_t i = 0, x, data_index = o->dargi;
    DBG("");
    DBG("def_validate_choices: type=%d data_index=%d word=%s nchoices=%d",
        o->construct->data[0].type, data_index, pl->word,
        o->construct->data[data_index].nchoices);
    DBG("def_validate_choices: construct=%s", o->construct->name);
    DBG("");
    /* aigh... tricky here.
     * if o->construct->data[0].type == UNIQUE then lookup which one we're on
     * to determine data_index */
    if(o->construct->data[0].type == UNIQUE) {
        if(o->dargi < 1)
            return;
        data_index = -1;
        for(i = 0; i < o->construct->dargs; i++) {
            if(strcmp(o->data[o->dargp]->word,
                    o->construct->data[i].word) == 0) {
                DBG("def_validate_choices: FOUND REALINDEX @ %d for %s",
                    i, o->data[o->dargp]->word);
                data_index = i;
                break;
            }
        }
        if(data_index == -1) {
            DBG("def_validate_choices: INVALID DATA INDEX");
            return;
        }
    }

    if(o->construct->data[data_index].nchoices) {
        DBG("def_validate_choices: VALIDATE");
        for(i = 1; i < o->construct->data[data_index].nchoices; i++) {
            if(strcmp(pl->word,
                    o->construct->data[data_index].choices[i]) == 0)
                return;
            x = strlen(o->construct->data[data_index].choices[i])-1;
            /* special checks for words in the form of 'default=' <word>= */
            if(o->construct->data[data_index].choices[i][x] == '=') {
                if(strncmp(pl->word, o->construct->data[data_index].choices[i],
                        x) == 0)
                    return;
            }
        }
        die(NULL, "ERROR: Invalid option `%s' on line %zd",
            pl->word, pl->line_number);
    } else {
        DBG("def_validate_choices: DO NOT VALIDATE");
    }
}

static void def_insert_new_data_line(struct parse_list *pl)
{
    struct object *o = objects_tail;
    size_t i = 0;
    int found = 0;

    /* first time for this object getting a data line */
    if(!o->data) {
        DBG("def_insert_new_data_line: 0 %s", pl->word);
        o->data = malloc_or_die(sizeof(*o->data)*2, "object data");
        o->dargp = 0;
    } else {
        DBG("def_insert_new_data_line: %d %s", o->dargp+1, pl->word);
        o->data = realloc_or_die(o->data, sizeof(*o->data)*(o->dargp+2),
            "object data");
        o->dargp++;
    }
    o->data[o->dargp] = malloc_or_die(sizeof(*o->data[o->dargp]),
        "object data_section");
    o->data[o->dargp]->word = pl->word;
    o->data[o->dargp]->tail = o->data[o->dargp];
    o->dargi = 0;

    /* OK, now validate from data_def.type */
    if(o->construct->data[0].type == UNIQUE) {
        for(i = 0; i < o->construct->dargs; i++) {
            if(strcmp(pl->word, o->construct->data[i].word) == 0) {
                found = 1;
                break;
            }
        }
        if(!found)
            die(NULL, "ERROR: Invalid word `%s' on line %zd",
                pl->word, pl->line_number);
    } else {
        def_validate_choices(o, pl);
    }
}

static void def_insert_data_arg(struct parse_list *pl)
{
    struct data_section *cur = objects_tail->data[objects_tail->dargp]->tail;
    struct object *o = objects_tail;
    DBG("def_insert_data_arg: %s", pl->word);
    cur->next = malloc_or_die(sizeof(*cur->next), "object data_section");
    cur->next->word = pl->word;
    objects_tail->data[objects_tail->dargp]->tail = cur->next;
    objects_tail->dargi++;
    def_validate_choices(o, pl);
}

void def_dump_it_all()
{
#ifdef DEBUG
    struct object *cur = objects;
    struct data_section *ds;
    int i = 0;
    int x = 0, y = 0;
    while(cur) {
        printf("construct_def: %s\n", cur->construct->name);
        for(i = 0; i < cur->construct->nargs; i++)
            printf("\targ[%d]=%s : %s\n",
                i, cur->construct->args[i].arg, cur->args[i]->arg);
        /* foreach data arg */
        for(x = 0; x < cur->dargp+1; x++) {
            ds = cur->data[x];
            /* foreach arg param */
            y = 0;
            while(ds) {
                if(cur->construct->data[0].type ==  ALLARG)
                    printf("\t\t*darg[%d][%s] : ",
                        x, cur->construct->data[0].word);
                else if(cur->construct->data[0].type == UNIQUE)
                    printf("\t\t+darg[%d][%s] : ", x, cur->data[x]->word);
                else
                    printf("\t\t-darg[%d][%s] : ",
                        x, cur->construct->data[y].word);
                printf("%s\n", ds->word);
                ds = ds->next;
                y++;
            }
        }
        cur = cur->next;
    }
#endif
}
void def_push(struct parse_list *pl)
{
    static enum parse_state_t prev_state = START;
    static size_t prev_line_number = 0;
    static int word_pos = 0;
    static int data_pos = 0;

    DBG("WORD_POS: %d", word_pos);
    switch(pl->state) {

        case WORD:
            /* got a construct def/new object */
            if(word_pos < 1)
                def_insert_new_object(pl);
            else
                def_insert_construct_arg(pl);
            word_pos++;
            data_pos = 0;
            break;

        case DATA:
            if(prev_state == WORD && objects_tail->construct->nargs > 0 &&
                    word_pos < 2) {
                die(NULL,
                    "ERROR: Expected argument on line %zd for `%s' construct.",
                    prev_line_number, objects_tail->construct->name);
            }
            if(data_pos < 1 || prev_line_number != pl->line_number) {
                data_pos = 1;
                def_insert_new_data_line(pl);
            } else {
                def_insert_data_arg(pl);
            }
            word_pos = 0;
            break;

        default:
            die(NULL, "ERROR: Invalid parse state %d at line %zd by word `%s'",
                pl->state, pl->line_number, pl->word);
    }
    prev_line_number = pl->line_number;
    prev_state = pl->state;
}


