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
#include <aws/cal/ecc.h>
#include <aws/cal/hash.h>

#define CHECK_AND_RETURN_FAILED(COND) \
    if (COND != AWS_OP_SUCCESS) { \
        return AWS_OP_ERR; \
    }

extern NONDET_FN_ATTR int64_t nd_int64_t(void);
#define MAX_ITERATION_BOUND 10

int key_derivation(struct aws_allocator *allocator,
                   enum aws_ecc_curve_name curve_name,
                   struct aws_byte_cursor *private_key,
                   struct aws_byte_cursor *expected_pub_x,
                   struct aws_byte_cursor *expected_pub_y) {
    
    struct aws_ecc_key_pair *private_key_pair =
      aws_ecc_key_pair_new_from_private_key(allocator, curve_name, private_key);

    if (!private_key_pair) {
    return AWS_OP_ERR;
    }

    int error = aws_ecc_key_pair_derive_public_key(private_key_pair);

    /* this isn't supported on Apple platforms, since AFAIK it isn't possible */
    if (error) {
        return AWS_OP_ERR;
    }

    struct aws_byte_cursor pub_x;
    struct aws_byte_cursor pub_y;
    
    aws_ecc_key_pair_get_public_key(private_key_pair, &pub_x, &pub_y);
    
    return AWS_OP_SUCCESS;
}

int ecdsa_import_asn1_key_pair(
    struct aws_allocator *allocator,
    struct aws_byte_cursor *asn1_cur,
    enum aws_ecc_curve_name expected_curve_name) {


    struct aws_ecc_key_pair *imported_key =
        aws_ecc_key_pair_new_from_asn1(allocator, asn1_cur);
    if (!imported_key || imported_key->curve_name != expected_curve_name) {
        return AWS_OP_ERR;
    }

    size_t msg_sz = nd_size_t();
    assume(msg_sz < MAX_SIZE);
    uint8_t *message = bounded_malloc_havoc(sizeof(uint8_t) * msg_sz);
    struct aws_byte_cursor message_input =
        aws_byte_cursor_from_array(message, sizeof(message));
    uint8_t hash[AWS_SHA256_LEN];
    memhavoc(hash, AWS_SHA256_LEN);
    struct aws_byte_buf hash_value =
        aws_byte_buf_from_empty_array(hash, sizeof(hash));

    size_t signature_size = aws_ecc_key_pair_signature_length(imported_key);

    struct aws_byte_buf signature_buf;
    aws_byte_buf_init(&signature_buf, allocator, signature_size);

    struct aws_byte_cursor hash_cur = aws_byte_cursor_from_buf(&hash_value);
    CHECK_AND_RETURN_FAILED(
        aws_ecc_key_pair_sign_message(imported_key, &hash_cur, &signature_buf));

    struct aws_byte_cursor signature_cur =
        aws_byte_cursor_from_buf(&signature_buf);
    CHECK_AND_RETURN_FAILED(aws_ecc_key_pair_verify_signature(
        imported_key, &hash_cur, &signature_cur));

    aws_byte_buf_clean_up(&signature_buf);
    aws_ecc_key_pair_release(imported_key);
    
    return AWS_OP_SUCCESS;
}

int main() {
    /* data structure */
    struct aws_allocator *allocator = sea_allocator();
    enum aws_ecc_curve_name curve_name = nd_int();
    assume(AWS_CAL_ECDSA_P256 <= curve_name);
    assume(curve_name <= AWS_CAL_ECDSA_P384);

    size_t encoded_sz = aws_ecc_key_coordinate_byte_size_from_curve_name(curve_name);
    /* assume preconditions */

    uint8_t *d = bounded_malloc_havoc(sizeof(uint8_t) * encoded_sz);
    struct aws_byte_cursor private_key =
        aws_byte_cursor_from_array(d, encoded_sz);

    uint8_t *x = bounded_malloc_havoc(sizeof(uint8_t) * encoded_sz);

    struct aws_byte_cursor pub_x = aws_byte_cursor_from_array(x, encoded_sz);

    uint8_t *y = bounded_malloc_havoc(sizeof(uint8_t) * encoded_sz);

    struct aws_byte_cursor pub_y = aws_byte_cursor_from_array(y, encoded_sz);

    CHECK_AND_RETURN_FAILED(
        key_derivation(allocator, curve_name, &private_key, &pub_x, &pub_y));

    uint8_t *asn1_encoded_key_raw =
        bounded_malloc_havoc(sizeof(uint8_t) * encoded_sz);

    struct aws_byte_cursor asn1_encoded_key = aws_byte_cursor_from_array(
        asn1_encoded_key_raw, sizeof(asn1_encoded_key_raw));
    CHECK_AND_RETURN_FAILED(ecdsa_import_asn1_key_pair(
        allocator, &asn1_encoded_key, curve_name));

    /* operation under verification */

    /* assertions */

    return 0;
}