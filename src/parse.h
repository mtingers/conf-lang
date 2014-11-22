#ifndef __PARSE_H
#define __PARSE_H

enum parse_state_t {
    START,
    WORD,
    SPACE,
    DATA,
    COMMENT,
    BRACE_OPEN,
    BRACE_CLOSE,
    QUOTE,
    NEWLINE,
};

/* The parse list contains the words and other syntax captured during the input
 * phase */
struct parse_list {
    char *word;
    size_t line_number;
    enum parse_state_t state;
    struct parse_list *next;
};

struct parse_list *parse(const char *path);

#endif

