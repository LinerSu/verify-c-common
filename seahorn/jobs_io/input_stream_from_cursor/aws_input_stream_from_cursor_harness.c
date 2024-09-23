/*
 * stream IO in aws only consider input stream either from a cursor or a 
 * file content.
 */

#include <seahorn/seahorn.h>
#include <aws/io/stream.h>
#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>

extern NONDET_FN_ATTR int64_t nd_int64_t(void);
#define MAX_ITERATION_BOUND 5

void initialize_byte_buffer_based_on_c_str(struct aws_byte_buf *const buf, size_t *sz) {
    size_t cap = nd_size_t();
    assume(cap <= MAX_BUFFER_SIZE);

    CRAB_ASSUME(cap > 1);

    buf->len = 0;
    buf->capacity = cap;
    buf->buffer = bounded_malloc_havoc(cap * sizeof(*(buf->buffer)));
    size_t idx = nd_size_t();
    assume(idx < cap);
    uint8_t val = nd_uint8_t();
    assume(val != 0);
    buf->buffer[idx] = val;
    // for (size_t i = 0; i < len - 1; ++i) {
    //     uint8_t val = nd_uint8_t();
    //     assume(val > 0);
    //     cur->ptr[i] = val;
    // }
    buf->buffer[cap - 1] = 0;
    *sz = cap - 1;
}

int main() {
    /* data structure */
    struct aws_byte_cursor *cur = malloc(sizeof(struct aws_byte_cursor));
    initialize_byte_cursor(cur);

    struct aws_allocator *allocator = sea_allocator();
    /* assume preconditions */
    assume(aws_byte_cursor_is_valid(cur));

    /* operation under verification */
    struct aws_input_stream *stream = aws_input_stream_new_from_cursor(allocator, cur);

    if (stream) {
        stream = aws_input_stream_acquire(stream);
        size_t n = MAX_ITERATION_BOUND;
        for (unsigned i = 0; i < n; i++) {
            /* Seek to BEGIN + offset. Read some bytes */
            size_t read_sz = 0;
            struct aws_byte_buf read_buf;
            initialize_byte_buffer_based_on_c_str(&read_buf, &read_sz);

            int64_t offset = nd_int64_t();
            assume(offset < read_sz);
            if (aws_input_stream_seek(stream, offset, AWS_SSB_BEGIN) == AWS_OP_SUCCESS) {
                aws_input_stream_read(stream, &read_buf);
            }
        }
        stream = aws_input_stream_release(stream);
    }
    /* assertions */

    return 0;
}