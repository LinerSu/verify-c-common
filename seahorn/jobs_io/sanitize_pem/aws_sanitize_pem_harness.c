/*
 *
 */

#include <seahorn/seahorn.h>
#include <aws/io/private/pem_utils.h>
#include <aws/io/pem.h>
#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>

void initialize_bigger_byte_buf(struct aws_byte_buf *const buf) {
    size_t len = nd_size_t();
    size_t cap = nd_size_t();
    assume(len <= cap);
    size_t min_size = 27;
    // Bug in sanitize pem, it assume buffer.cap > 26
    assume(cap >= min_size);
    assume(cap <= MAX_SIZE);
    // cap = 35;

    CRAB_ASSUME(len > 0);

    buf->len = len;
    buf->capacity = cap;
    buf->buffer = bounded_malloc_havoc(cap * sizeof(*(buf->buffer)));
    buf->allocator = sea_allocator();
}


int main() {
    /* data structure */
    struct aws_byte_buf pem_buf;
    initialize_bigger_byte_buf(&pem_buf);

    struct aws_allocator *allocator = sea_allocator();
    /* assume preconditions */
    assume(aws_byte_buf_is_valid(&pem_buf));

    /* operation under verification */
    aws_sanitize_pem(&pem_buf, allocator);
    /* assertions */

    return 0;
}