#include "tpl.h"
#include "conf-lang.h"
#include <ctype.h>

/* The main buffer for storing words during the input phase */
static const size_t PARSE_MAX_WORD_LEN  = 1048576; /* 1MB */
static char *tpl_buffer               = NULL; /* size=PARSE_MAX_WORD_LEN */
static size_t line_num                  = 1;
static size_t char_pos                  = 0;
static size_t tpl_buf_len             = 0;

static enum tpl_state_t state            = START;
static enum tpl_state_t state_prev       = START;
static enum tpl_state_t state_capture    = WORD;

static struct tpl_list *plist, *plist_tail = NULL;

static inline void state_set(int s)
{
    state_prev = state;
    state = s;
}

static struct tpl_list *tpl_list_new()
{
    struct tpl_list *pl = NULL;
    if(!(pl = malloc(sizeof(*pl))))
        die("malloc", "ERROR: Failed to allocate tpl list structure");
    pl->word = NULL;
    pl->next = NULL;
    pl->line_number = 0;
    return pl;
}

static void allocate_buffers()
{
    tpl_buffer = malloc(sizeof(*tpl_buffer)*PARSE_MAX_WORD_LEN+1);
    if(!tpl_buffer)
        die("malloc", "ERROR: Failed to allocate tpl buffer");
    plist = tpl_list_new();
    plist_tail = plist;
}

struct tpl_list *tpl(const char *path)
{
    char *p = file_get(path);

    int brace_open_i = 0, brace_close_i = 0;
    if(!p)
        die("fopen", "ERROR: Failed to read file `%s'", path);

    allocate_buffers();

    DBG("Begin parsing...");
    do {
        switch(*p) {
            case '{':
                if(++brace_open_i > 2)
                    die(NULL, "ERROR(line=%zd, char=%zd): '}' invalid { count.", line_num, char_pos);
                tpl_list_insert();
                state_set(BRACE_OPEN);
                state_capture = DATA;
                break;
            case '}':
                brace_close_i++;
                if(state != NEWLINE)
                    die(NULL, "ERROR(line=%zd, char=%zd): '}' must be preceded by a newline.", line_num, char_pos);
                tpl_list_insert();
                state_set(BRACE_CLOSE);
                state_capture = WORD;
                break;
            case '\n':
                tpl_list_insert();
                line_num++;
                state_set(NEWLINE);
                break;
            case '#':
                if(state != COMMENT) {
                    if(state != SPACE && state != NEWLINE)
                        die(NULL, "ERROR(line=%zd, char=%zd): A comment must be preceded by a space or newline.",
                            line_num, char_pos);
                    if(state != NEWLINE && state_capture == WORD)
                        die(NULL, "ERROR(line=%zd, char=%zd): A comment cannot be preceded by space here.",
                            line_num, char_pos);
                    state_set(COMMENT);
                }
                break;
            case '\'':
                if(state != NEWLINE && state != SPACE && state != QUOTE)
                    die(NULL, "ERROR(line=%zd, char=%zd): A quote must be preceded by a space or newline.",
                        line_num, char_pos);

                /* prev state was quote so this is the closing quote */
                if(state == QUOTE) {
                    state = state_prev;
                    state_prev = QUOTE;
                } else {
                    state_prev = state;
                    state = QUOTE;
                    tpl_list_insert();
                }
                break;
        /* handle non-integer constants here or die */
        default:
            if(state == COMMENT)
                break;

            /* capture words here */
            if(('=' == *p || '-' == *p || '_' == *p || isalnum(*p)) || state == QUOTE) {
                if(state == BRACE_CLOSE)
                    die(NULL, "ERROR(line=%zd, char=%zd): '}' must be followed by a newline.", line_num, char_pos);
                if(state == BRACE_OPEN)
                    die(NULL, "ERROR(line=%zd, char=%zd): '{' must be followed by a newline.", line_num, char_pos);
                if(state == COMMENT)
                    break;
                if(state != NEWLINE && state_prev == QUOTE)
                    die(NULL, "ERROR(line=%zd, char=%zd): Cannot append to quoted word.",
                        line_num, char_pos, state, state_prev);
                /* state reset */
                if(state != WORD && state != QUOTE) {
                    tpl_list_insert();
                }
                tpl_buffer[tpl_buf_len]   = *p;
                tpl_buffer[tpl_buf_len+1] = '\0';
                tpl_buf_len++;
                if(state != QUOTE)
                    state_set(WORD);
                break;
            }
            if(isblank(*p)) {
                state_set(SPACE);
                break;
            }
            if(isascii(*p))
                die(NULL, "ERROR(line=%zd, char=%zd): Invalid character `%c'. You may need to quote it.", line_num, char_pos, *p);
            else
                die(NULL, "ERROR(line=%zd, char=%zd): Invalid character `0x%02x'. You may need to quote it.", line_num, char_pos, *p&0xff);
        }
        p++, char_pos++;
    } while(*p != '\0');
    DBG("OK: File validated");
    //tpl_list_dump();
    free(tpl_buffer);
    tpl_buffer = NULL;
    return plist;
}



int main()
{
    return 0;
}
