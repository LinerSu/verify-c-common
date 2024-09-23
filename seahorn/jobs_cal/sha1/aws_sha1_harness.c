/*
 * ECC (Elliptic Curve Cryptography) is a key pair based cryptography. Each pair
 * consists of a public key and a corresponding private key.
 */

#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>

#include <seahorn/seahorn.h>
#include <aws/cal/cal.h>
#include <aws/cal/hash.h>

#define CHECK_AND_RETURN_FAILED(COND) \
    if (COND != AWS_OP_SUCCESS) { \
        return AWS_OP_ERR; \
    }

extern NONDET_FN_ATTR int64_t nd_int64_t(void);

void initialize_bigger_byte_cursor(struct aws_byte_cursor *const cur) {
    size_t len = nd_size_t();
    assume(len <= MAX_SIZE);

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

int verify_hash(struct aws_allocator *allocator, struct aws_byte_cursor *input,
    struct aws_byte_cursor *expected) {
    /* test all possible segmentation lengths from 1 byte at a time to the entire
     * input. Using a do-while so that we still do 1 pass on 0-length input */
    size_t advance_i = 1;
    do {
        uint8_t output[128] = {0};
        struct aws_byte_buf output_buf = aws_byte_buf_from_empty_array(output, AWS_ARRAY_SIZE(output));

        struct aws_hash *hash = aws_sha1_new(allocator);
        if (!hash) {
            return AWS_OP_ERR;
        }
        struct aws_byte_cursor input_cpy = *input;
        while (input_cpy.len > 0) {
            size_t max_advance = aws_min_size(input_cpy.len, advance_i);
            struct aws_byte_cursor segment = aws_byte_cursor_from_array(input_cpy.ptr, max_advance);
            CHECK_AND_RETURN_FAILED(aws_hash_update(hash, &segment));
            aws_byte_cursor_advance(&input_cpy, max_advance);
            // if (max_advance < input_cpy.len) {
            //     input_cpy.ptr = input_cpy.ptr + max_advance;
            //     input_cpy.len -= max_advance;
            // }
        }
        size_t truncation_size = expected->len;
        CHECK_AND_RETURN_FAILED(aws_hash_finalize(hash, &output_buf, truncation_size));

        aws_hash_destroy(hash);
    } while (++advance_i <= input->len);
    return AWS_OP_SUCCESS;
}

int sha1_nist_1(struct aws_allocator *allocator) {
    struct aws_byte_cursor input;
    initialize_bigger_byte_cursor(&input);
    assume(aws_byte_cursor_is_valid(&input));
    uint8_t expected[AWS_SHA1_LEN];
    memhavoc(expected, sizeof(AWS_SHA1_LEN));
    struct aws_byte_cursor expected_buf = aws_byte_cursor_from_array(expected, sizeof(expected));

    return verify_hash(allocator, &input, &expected_buf);
}

int sha1_nist_2(struct aws_allocator *allocator) {
    struct aws_hash *hash = aws_sha1_new(allocator);
    if (!hash) {
        return AWS_OP_ERR;
    }

    struct aws_byte_cursor input;
    initialize_bigger_byte_cursor(&input);
    assume(aws_byte_cursor_is_valid(&input));

    for (size_t i = 0; i < MAX_SIZE; ++i) {
        CHECK_AND_RETURN_FAILED(aws_hash_update(hash, &input));
    }

    uint8_t output[AWS_SHA1_LEN] = {0};
    struct aws_byte_buf output_buf = aws_byte_buf_from_array(output, sizeof(output));
    output_buf.len = 0;
    CHECK_AND_RETURN_FAILED(aws_hash_finalize(hash, &output_buf, 0));

    uint8_t expected[AWS_SHA1_LEN];
    memhavoc(expected, sizeof(AWS_SHA1_LEN));
    struct aws_byte_cursor expected_buf = aws_byte_cursor_from_array(expected, sizeof(expected));
    return AWS_OP_SUCCESS;
}

int verify_sha_compute(struct aws_allocator *allocator) {
    struct aws_byte_cursor input;
    initialize_bigger_byte_cursor(&input);
    assume(aws_byte_cursor_is_valid(&input));

    struct aws_byte_buf digest_size_buf;

    CHECK_AND_RETURN_FAILED(aws_byte_buf_init(&digest_size_buf, allocator, AWS_SHA1_LEN));

    uint8_t expected[AWS_SHA1_LEN];
    memhavoc(expected, sizeof(AWS_SHA1_LEN));

    CHECK_AND_RETURN_FAILED(aws_sha1_compute(allocator, &input, &digest_size_buf, 0));

    return AWS_OP_SUCCESS;
}

int main() {
    /* data structure */
    struct aws_allocator *allocator = sea_allocator();
    /* assume preconditions */
    /* operation under verification */
    CHECK_AND_RETURN_FAILED(sha1_nist_1(allocator));
    CHECK_AND_RETURN_FAILED(sha1_nist_2(allocator));
    CHECK_AND_RETURN_FAILED(verify_sha_compute(allocator));

    /* assertions */
    return 0;
}