
#include "conf-lang.h"
#include "parse.h"
#include "def.h"

/* The flow of phases of executing an input file
 *
 * A. Input
 *      1) Read file
 *      2) Parse file and build token list
 * B. Control Structures
 *      1) Read token list and build control structures
 *      2)
 * C.
 *
 */

void foreach_plist(struct parse_list *pl)
{
    struct parse_list *cur = pl;
    while(cur) {
        DBG("foreach_plist: word=%s line_number=%zd state=%zd",
            cur->word, cur->line_number, cur->state);
        def_push(cur);
        cur = cur->next;
    }
}

int main(int argc, char **argv)
{
    struct parse_list *pl;
    pl = parse(argv[1]);
    foreach_plist(pl);
    def_dump_it_all();
    fflush(stdout);
    fflush(stderr);
    return 0;
}
