#ifndef __TPL_H
#define __TPL_H

enum tpl_state_t {
    START,
    WORD,
    SPACE,
    DATA,
    COMMENT,
    BRACE_OPEN1,
    BRACE_OPEN2,
    BRACE_CLOSE1,
    BRACE_CLOSE2,
    QUOTE,
    NEWLINE,
    PER_OPEN,
    PER_CLOSE,
};

/* The tpl list contains the words and other syntax captured during the input
 * phase */
struct tpl_list {
    char *word;
    size_t line_number;
    enum tpl_state_t state;
    struct tpl_list *next;
};

struct tpl_list *tpl(const char *path);

#endif

