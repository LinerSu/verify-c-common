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

void initialize_bigger_byte_cursor(struct aws_byte_cursor *const cur) {
    size_t len = nd_size_t();
    assume(len <= MAX_BUFFER_SIZE);

    CRAB_ASSUME(len > 1);

    cur->len = len;
    cur->ptr = bounded_malloc_havoc(len * sizeof(*(cur->ptr)));
    size_t idx = nd_size_t();
    assume(idx < len);
    uint8_t val = nd_uint8_t();
    assume(val != 0);
    cur->ptr[idx] = val;
    // for (size_t i = 0; i < len - 1; ++i) {
    //     uint8_t val = nd_uint8_t();
    //     assume(val > 0);
    //     cur->ptr[i] = val;
    // }
    cur->ptr[len - 1] = 0;
}

int main() {
    /* data structure */
    struct aws_allocator *allocator = sea_allocator();
    struct aws_byte_cursor pem_data;
    initialize_bigger_byte_cursor(&pem_data);
    struct aws_array_list output_list;

    /* assume preconditions */

    /* operation under verification */
    aws_pem_objects_init_from_file_contents(&output_list, allocator, pem_data);
    /* assertions */

    return 0;
}