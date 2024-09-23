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
#include <aws/cal/hmac.h>

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

int wrap_memcmp(const void *expected, size_t expected_size, const void *got,
                size_t got_size) {
    if (expected_size != got_size || !expected || !got) {
        return AWS_OP_ERR;
    }
    const uint8_t *expected_char = (const uint8_t *)(expected);
    const uint8_t *got_char = (const uint8_t *)(got);
    sassert(sea_is_dereferenceable(expected_char, expected_size));
    sassert(sea_is_dereferenceable(got_char, got_size));
    if (memcmp(expected_char, got_char, got_size) != 0) {
        return AWS_OP_ERR;
    }
    return AWS_OP_SUCCESS;
}

int verify_hmac(struct aws_allocator *allocator, struct aws_byte_cursor *input,
                struct aws_byte_cursor *secret,
                struct aws_byte_cursor *expected, aws_hmac_new_fn *new_fn) {
    /* test all possible segmentation lengths from 1 byte at a time to the entire
    * input. Using a do-while so that we still do 1 pass on 0-length input */
    size_t advance_i = 1;
    do {
        uint8_t output[128] = {0};
        struct aws_byte_buf output_buf =
            aws_byte_buf_from_empty_array(output, AWS_ARRAY_SIZE(output));

        struct aws_hmac *hmac = new_fn(allocator, secret);
        if (!hmac) {
        return AWS_OP_ERR;
        }

        struct aws_byte_cursor input_cpy = *input;
        while (input_cpy.len) {
        size_t max_advance = aws_min_size(input_cpy.len, advance_i);
        struct aws_byte_cursor segment =
            aws_byte_cursor_from_array(input_cpy.ptr, max_advance);
        CHECK_AND_RETURN_FAILED(aws_hmac_update(hmac, &segment));
        aws_byte_cursor_advance(&input_cpy, max_advance);
        }

        size_t truncation_size = expected->len;

        CHECK_AND_RETURN_FAILED(
            aws_hmac_finalize(hmac, &output_buf, truncation_size));
        // CHECK_AND_RETURN_FAILED(wrap_memcmp(expected->ptr, expected->len,
        // output_buf.buffer, output_buf.len));

        aws_hmac_destroy(hmac);
    } while (++advance_i <= input->len);

    return AWS_OP_SUCCESS;
}

int sha256_hmac_nist_1(struct aws_allocator *allocator) {
    struct aws_byte_cursor input;
    initialize_bigger_byte_cursor(&input);
    assume(aws_byte_cursor_is_valid(&input));

    uint8_t expected[AWS_SHA256_HMAC_LEN];
    memhavoc(expected, sizeof(AWS_SHA256_HMAC_LEN));

    size_t bounded_sz = nd_size_t();
    assume(bounded_sz <= MAX_BUFFER_SIZE);
    uint8_t *secret = bounded_malloc_havoc(bounded_sz);
    struct aws_byte_cursor secret_buf =
        aws_byte_cursor_from_array(secret, sizeof(secret));
    struct aws_byte_cursor expected_buf =
        aws_byte_cursor_from_array(expected, sizeof(expected));

    return verify_hmac(allocator, &input, &secret_buf, &expected_buf,
                       aws_sha256_hmac_new);
}

int verify_sha256_hmac_compute(struct aws_allocator *allocator) {
    size_t bounded_sz = nd_size_t();
    assume(bounded_sz <= MAX_BUFFER_SIZE);
    uint8_t *secret = bounded_malloc_havoc(bounded_sz);
    struct aws_byte_cursor secret_buf = aws_byte_cursor_from_array(secret, sizeof(secret));

    struct aws_byte_cursor input_buf;
    initialize_bigger_byte_cursor(&input_buf);
    assume(aws_byte_cursor_is_valid(&input_buf));

    uint8_t output[AWS_SHA256_HMAC_LEN] = {0};
    struct aws_byte_buf output_buf = aws_byte_buf_from_array(output, sizeof(output));
    output_buf.len = 0;

    struct aws_hmac *hmac = aws_sha256_hmac_new(allocator, &secret_buf);
    if (!hmac) {
        return AWS_OP_ERR;
    }
    CHECK_AND_RETURN_FAILED(aws_hmac_update(hmac, &input_buf));
    CHECK_AND_RETURN_FAILED(aws_hmac_finalize(hmac, &output_buf, 0));

    CHECK_AND_RETURN_FAILED(aws_sha256_hmac_compute(allocator, &secret_buf, &input_buf, &output_buf, 0));

    aws_hmac_destroy(hmac);
    return AWS_OP_SUCCESS;
}

int main() {
    /* data structure */
    struct aws_allocator *allocator = sea_allocator();
    /* assume preconditions */
    /* operation under verification */
    CHECK_AND_RETURN_FAILED(sha256_hmac_nist_1(allocator));
    CHECK_AND_RETURN_FAILED(verify_sha256_hmac_compute(allocator));

    /* assertions */

    return 0;
}