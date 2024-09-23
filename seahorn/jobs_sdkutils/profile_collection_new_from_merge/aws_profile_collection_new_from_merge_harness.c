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
    struct aws_allocator *allocator = sea_allocator();
    struct aws_string *config_contents = ensure_string_is_allocated_bounded_length(MAX_BUFFER_SIZE);
    struct aws_string *credentials_contents = ensure_string_is_allocated_bounded_length(MAX_BUFFER_SIZE);
    struct aws_byte_cursor config_cursor = aws_byte_cursor_from_string(config_contents);
    struct aws_byte_buf config_buffer;
    aws_byte_buf_init_copy_from_cursor(&config_buffer, allocator, config_cursor);

    struct aws_profile_collection *config_profile_collection =
        aws_profile_collection_new_from_buffer(allocator, &config_buffer, AWS_PST_CONFIG);

    aws_byte_buf_clean_up(&config_buffer);

    struct aws_byte_cursor credentials_cursor = aws_byte_cursor_from_string(credentials_contents);
    struct aws_byte_buf credentials_buffer;
    aws_byte_buf_init_copy_from_cursor(&credentials_buffer, allocator, credentials_cursor);

    struct aws_profile_collection *credentials_profile_collection =
        aws_profile_collection_new_from_buffer(allocator, &credentials_buffer, AWS_PST_CREDENTIALS);

    aws_byte_buf_clean_up(&credentials_buffer);

    struct aws_profile_collection *merged =
        aws_profile_collection_new_from_merge(allocator, config_profile_collection, credentials_profile_collection);
    
    sassert(merged != NULL);
    sassert(aws_profile_collection_get_profile_count(merged) == 1);

    return 0;
}