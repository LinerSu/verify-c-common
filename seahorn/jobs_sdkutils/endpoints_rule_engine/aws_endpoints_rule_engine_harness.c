/*
 *
 */

#include <seahorn/seahorn.h>
#include <aws/sdkutils/endpoints_rule_engine.h>
#include <aws/sdkutils/partitions.h>
#include <aws/sdkutils/private/endpoints_types_impl.h>
#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>

static int read_file_contents(
    struct aws_byte_buf *out_buf,
    struct aws_allocator *alloc,
    const struct aws_byte_cursor filename_cur) {

    aws_byte_buf_init(out_buf, alloc, MAX_BUFFER_SIZE);
    assume(aws_byte_buf_is_valid(out_buf));
    return AWS_OP_SUCCESS;
}

int main() {
    /* data structure */

    struct aws_allocator *allocator = sea_allocator();
    struct aws_byte_cursor file_name;
    initialize_byte_cursor(&file_name);

    /* assume preconditions */
    assume(aws_byte_cursor_is_valid(&file_name));
    aws_sdkutils_library_init(allocator);

    struct aws_byte_buf ruleset_file_path;
    if (aws_byte_buf_init_copy_from_cursor(&ruleset_file_path, allocator, aws_byte_cursor_from_c_str("valid-rules/")) != AWS_OP_SUCCESS) {
      return -1;
    }
    if (aws_byte_buf_append_dynamic(&ruleset_file_path, &file_name) != AWS_OP_SUCCESS) {
      return -1;
    }

    struct aws_byte_buf test_cases_file_path;
    if (aws_byte_buf_init_copy_from_cursor(
        &test_cases_file_path, allocator, aws_byte_cursor_from_c_str("test-cases/")) != AWS_OP_SUCCESS) {
      return -1;
    }

    if (aws_byte_buf_append_dynamic(&test_cases_file_path, &file_name) != AWS_OP_SUCCESS) {
      return -1;
    }

    struct aws_byte_buf ruleset_buf;
    if (read_file_contents(&ruleset_buf, allocator, aws_byte_cursor_from_buf(&ruleset_file_path)) != AWS_OP_SUCCESS) {
      return -1;
    }
    struct aws_byte_cursor ruleset_json = aws_byte_cursor_from_buf(&ruleset_buf);
    struct aws_endpoints_ruleset *ruleset = aws_endpoints_ruleset_new_from_string(allocator, ruleset_json);

    struct aws_byte_buf partitions_buf;
    if (read_file_contents(&partitions_buf, allocator, aws_byte_cursor_from_c_str("partitions.json")) != AWS_OP_SUCCESS) {
      return -1;
    }
    struct aws_byte_cursor partitions_json = aws_byte_cursor_from_buf(&partitions_buf);
    struct aws_partitions_config *partitions = aws_partitions_config_new_from_string(allocator, partitions_json);

    struct aws_endpoints_rule_engine *engine = aws_endpoints_rule_engine_new(allocator, ruleset, partitions);

    struct aws_byte_buf test_cases_buf;
    if (read_file_contents(&test_cases_buf, allocator, aws_byte_cursor_from_buf(&test_cases_file_path))) {
        return 0;
    }
    struct aws_byte_cursor test_cases_json = aws_byte_cursor_from_buf(&test_cases_buf);

    size_t max_json_array_sz = 1;
    // assume(max_json_array_sz < 10);

    /* perform operation under verification */
    for (size_t i = 0; i < max_json_array_sz; ++i) {
        struct aws_endpoints_request_context *context = aws_endpoints_request_context_new(allocator);

        struct aws_endpoints_resolved_endpoint *resolved_endpoint = NULL;

        if (aws_endpoints_rule_engine_resolve(engine, context, &resolved_endpoint) != AWS_OP_SUCCESS) {
            return -1;
        }

        if (resolved_endpoint != NULL) {
            /* assertions */
            sassert(aws_endpoints_resolved_endpoint_get_type(resolved_endpoint) == AWS_ENDPOINTS_RESOLVED_ENDPOINT);
            struct aws_byte_cursor url;
            sassert(aws_endpoints_resolved_endpoint_get_url(resolved_endpoint, &url) == AWS_OP_SUCCESS);

            struct aws_byte_cursor properties;
            sassert(aws_endpoints_resolved_endpoint_get_properties(resolved_endpoint, &properties) == AWS_OP_SUCCESS);

            const struct aws_hash_table *headers;
            sassert(aws_endpoints_resolved_endpoint_get_headers(resolved_endpoint, &headers) == AWS_OP_SUCCESS);
        }
    }

    return 0;
}