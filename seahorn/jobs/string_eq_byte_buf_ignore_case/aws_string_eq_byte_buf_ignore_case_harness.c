/*
 * 
 */

#include <aws/common/string.h>
#include <seahorn/seahorn.h>
#include <byte_buf_helper.h>
#include <string_helper.h>
#include <utils.h>
#include <stddef.h>

int main() {
#ifdef __CRAB__
  struct aws_string *str =
      ensure_string_is_allocated_bounded_length(MAX_STRING_LEN);
#else
  struct aws_string *str =
      nd_bool() ? ensure_string_is_allocated_bounded_length(MAX_STRING_LEN)
                : NULL;
#endif
  struct aws_byte_buf buf;
  initialize_byte_buf(&buf);

  assume(aws_byte_buf_is_valid(&buf));

#ifdef __CRAB__
  bool nondet_parameter = true;
#else
  bool nondet_parameter = nd_bool();
#endif
  if (aws_string_eq_byte_buf_ignore_case(str, nondet_parameter ? &buf : NULL) &&
      str) {
    sassert(aws_string_is_valid(str));
    if (nondet_parameter) {
      sassert(str->len == buf.len);
    }
  }

  sassert(aws_byte_buf_is_valid(&buf));

  return 0;
}
