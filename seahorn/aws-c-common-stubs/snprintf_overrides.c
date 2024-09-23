/**

    Implementation of snprintf()

 */
#include <nondet.h>
#include <seahorn/seahorn.h>
#include <stdio.h>
extern char nd_char(void);

int snprintf(char *str, size_t size, const char *format, ...) {
    // avoid unhandled instruction for snprintf
    sassert(sea_is_dereferenceable(str, size));
    memhavoc(str, size);
    size_t idx = nd_size_t();
    assume(idx < size);
    char val = nd_char();
    assume(val != '\0');
    str[idx] = val;
    // treat as noop
    return 1;
}