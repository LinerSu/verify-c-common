/**
 *
 */

#include <seahorn/seahorn.h>
#include <aws/common/array_list.h>
#include <aws/common/byte_buf.h>
#include <proof_allocators.h>
#include <byte_buf_helper.h>
#include <utils.h>
#include <bounds.h>

int main() {
    /* parameters */
    struct aws_byte_cursor cur;
    initialize_byte_cursor(&cur);
    char split_on = 'a';
    size_t num = nd_size_t();
    size_t max_splits = nd_size_t();
    struct aws_array_list lst = {0};

    /* assumptions */
    assume(aws_byte_cursor_is_bounded(&cur, 5));
    assume(aws_byte_cursor_is_valid(&cur));
    assume(0 < max_splits && max_splits < 5);
    assume(num < sea_max_array_list_len());
    CRAB_ASSUME(num > 0);
    if (aws_array_list_init_dynamic(&lst, sea_allocator(), num, sizeof(struct aws_byte_cursor))) {
        return -1;
    }
    assume(aws_array_list_is_valid(&lst));

    /* save current state of the data structure */
    sea_tracking_on();
    sea_reset_modified((char *)cur.ptr);
    size_t idx = 0;

    /* operation under verification */
    while (idx < num) {
        struct aws_byte_cursor substr = {0};

        if (AWS_UNLIKELY(aws_array_list_push_back(&lst, (const void *)&substr))) {
            return -1;
        }
        sassert(sea_is_dereferenceable(lst.data + idx * sizeof(struct aws_byte_cursor), sizeof(struct aws_byte_cursor)));
        ++idx;
    }

    if (aws_byte_cursor_split_on_char_n(&cur, split_on, max_splits, &lst) ==
        AWS_OP_SUCCESS) {
       sassert(aws_array_list_is_valid(&lst));
       size_t idx = nd_size_t();
       size_t len = aws_array_list_length(&lst);
       assume(idx < len);
       sassert(sea_is_dereferenceable(lst.data + idx * sizeof(struct aws_byte_cursor), sizeof(struct aws_byte_cursor)));
    }



    /* assertions */
    sassert(aws_byte_cursor_is_valid(&cur));
    if (cur.len > 0) {
        sassert(!sea_is_modified((char *)cur.ptr));
    }
    return 0;
}