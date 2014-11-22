
/* To make this parser efficient, simple and subject to as few states as possible,
 * it runs by strict rules with minimal formatting variation.
 * Parser rules:
 *  [[space]<#comment><newline>]
 *  [quote]<word>[quote]<space>[[quote]<word>[quote]<space>]{<newline>
 *  [space][quote]<word>[quote][<space>[quote]<word>[quote]...]<newline>
 *  [[space]<#comment><newline>]
 *  ...
 *  }<newline>
 *
 * quote    - Any type quote of ' or ".
 * space    - Any type of space defined by isblank() in ctype.
 * word     - A series of characters all of which pass isalnum() and isascii() test.
 * newline  - Recognized as '\n' only (for now)
 * comment  - The '#' character
 * braces   - The chars '{' and '}' represent open and close of a data section
 *
 * NOTE: The format consists of a declaration section and a data section.
 *
 * NOTE: Magic numbers such as PARSE_MAX_WORD_LEN can be hit. PARSE_MAX_WORD_LEN
 *       is set quite large and should never be reached under normal circumstances.
 *
 * NOTE: Comments can be placed only on their own line
 *
 * EXAMPLE:
 * # Declaration section: the database connection
 * model-connection {
 *      # Data section
 *      name foo
 *      user bar
 *      pass "baz w/space quoted"
 *      host localhost
 *      # comment here at the end
 * }
 */

#include "conf-lang.h"
#include "parse.h"
#include <ctype.h>

static void allocate_buffers();
static struct parse_list *parse_list_new();

/* The main buffer for storing words during the input phase */
static const size_t PARSE_MAX_WORD_LEN  = 1048576; /* 1MB */
static char *parse_buffer               = NULL; /* size=PARSE_MAX_WORD_LEN */
static size_t line_num                  = 1;
static size_t char_pos                  = 0;
static size_t parse_buf_len             = 0;

static enum parse_state_t state            = START;
static enum parse_state_t state_prev       = START;
static enum parse_state_t state_capture    = WORD;

static struct parse_list *plist, *plist_tail = NULL;

static void parse_list_insert()
{
    struct parse_list *cur = plist_tail;

    /* NEWLINE can be detected by line number changes */
    //if(state != NEWLINE && parse_buf_len < 1) return;
    if(parse_buf_len < 1) return;
    /* nothing inserted yet */
    if(!plist->word) {
        if(!(plist->word = malloc(sizeof(*plist->word)*parse_buf_len+1)))
            die("malloc", "ERROR: Failed to insert parse buffer");
        sprintf(plist->word, "%s", parse_buffer);
        plist->line_number = line_num;
        plist->state = state_capture;
        parse_buf_len = 0;
        parse_buffer[0] = '\0';
        //if(state == NEWLINE)
        //    plist->state = NEWLINE;
        return;
    }
    cur->next = parse_list_new();
    cur = cur->next;
    if(!(cur->word = malloc(sizeof(*cur->word)*parse_buf_len+1)))
        die("malloc", "ERROR: Failed to insert parse buffer");
    sprintf(cur->word, "%s", parse_buffer);
    cur->line_number = line_num;
    cur->state = state_capture;
    parse_buf_len = 0;
    parse_buffer[0] = '\0';
    //if(state == NEWLINE)
    //    cur->state = NEWLINE;
    plist_tail = cur;
}

/*
static void parse_list_dump()
{
    struct parse_list *cur = plist;
    while(cur) {
        printf("DUMP_PLIST: word=`%s', line=%zd, state=%d\n",
            cur->word, cur->line_number, cur->state);
        cur = cur->next;
    }
}
*/

static inline void state_set(int s)
{
    state_prev = state;
    state = s;
}

static struct parse_list *parse_list_new()
{
    struct parse_list *pl = NULL;
    if(!(pl = malloc(sizeof(*pl))))
        die("malloc", "ERROR: Failed to allocate parse list structure");
    pl->word = NULL;
    pl->next = NULL;
    pl->line_number = 0;
    return pl;
}

static void allocate_buffers()
{
    parse_buffer = malloc(sizeof(*parse_buffer)*PARSE_MAX_WORD_LEN+1);
    if(!parse_buffer)
        die("malloc", "ERROR: Failed to allocate parse buffer");
    plist = parse_list_new();
    plist_tail = plist;
}

struct parse_list *parse(const char *path)
{
    char *p = file_get(path);

    if(!p)
        die("fopen", "ERROR: Failed to read file `%s'", path);

    allocate_buffers();

    DBG("Begin parsing...");
    do {
        switch(*p) {
            case '{':
                if(state != SPACE)
                    die(NULL, "ERROR(line=%zd, char=%zd): '{' must be preceded by a space.", line_num, char_pos);
                parse_list_insert();
                state_set(BRACE_OPEN);
                state_capture = DATA;
                break;
            case '}':
                if(state != NEWLINE)
                    die(NULL, "ERROR(line=%zd, char=%zd): '}' must be preceded by a newline.", line_num, char_pos);
                parse_list_insert();
                state_set(BRACE_CLOSE);
                state_capture = WORD;
                break;
            case '\n':
                if(state == SPACE && state_prev != QUOTE)
                    die(NULL, "ERROR(line=%zd, char=%zd): Newline cannot be preceded by a space.", line_num, char_pos);
                if(state == WORD && state_capture == WORD)
                    die(NULL, "ERROR(line=%zd, char=%zd): Newline cannot be preceded by a word.", line_num, char_pos);
                parse_list_insert();
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
                    parse_list_insert();
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
                    parse_list_insert();
                }
                parse_buffer[parse_buf_len]   = *p;
                parse_buffer[parse_buf_len+1] = '\0';
                parse_buf_len++;
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
    //parse_list_dump();
    free(parse_buffer);
    parse_buffer = NULL;
    return plist;
}

