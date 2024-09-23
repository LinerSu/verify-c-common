/*
 *
 */

#include <seahorn/seahorn.h>
#include <aws/sdkutils/aws_profile.h>
#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>


int main() {

    /* data structure */
    struct aws_string *profile_contents = ensure_string_is_allocated_bounded_length(MAX_BUFFER_SIZE);
    struct aws_allocator *allocator = sea_allocator();
    enum aws_profile_source_type source = nd_int();

    struct aws_byte_cursor contents = aws_byte_cursor_from_string(profile_contents);
    struct aws_byte_buf buffer;
    initialize_byte_buf(&buffer);
    /* assume preconditions */
    assume(AWS_PST_NONE <= source);
    assume(source <= AWS_PST_CREDENTIALS);
    aws_byte_buf_init_copy_from_cursor(&buffer, allocator, contents);

    /* perform operation under verification */
    struct aws_profile_collection *profile_collection =
        aws_profile_collection_new_from_buffer(allocator, &buffer, source);
    /* assertions */

    aws_byte_buf_clean_up(&buffer);

    return 0;
}