/*
 *
 */

#include <aws/common/string.h>
#include <byte_buf_helper.h>
#include <seahorn/seahorn.h>
#include <stddef.h>
#include <string_helper.h>
#include <utils.h>

int main(void) {
  if (nd_bool()) {
    struct aws_string *str = ensure_string_is_allocated_nondet_length();
    aws_string_destroy(str);
  } else {
    struct aws_string *str = NULL;
    aws_string_destroy(str);
  }

  return 0;
}
