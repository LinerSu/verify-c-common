/*
 * ECC (Elliptic Curve Cryptography) is a key pair based cryptography. Each pair
 * consists of a public key and a corresponding private key.
 */

#include <byte_buf_helper.h>
#include <config.h>
#include <nondet.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <utils.h>

#include <aws/cal/cal.h>
#include <aws/cal/hash.h>
#include <aws/cal/rsa.h>
#include <aws/common/encoding.h>
#include <seahorn/seahorn.h>

#define CHECK_AND_RETURN_FAILED(COND)                                          \
  if (COND != AWS_OP_SUCCESS) {                                                \
    return AWS_OP_ERR;                                                         \
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

static int s_byte_buf_decoded_from_base64_cur(struct aws_allocator *allocator,
                                              struct aws_byte_cursor cur,
                                              struct aws_byte_buf *out) {
  size_t decoded_length = 0;
  CHECK_AND_RETURN_FAILED(
      aws_base64_compute_decoded_len(&cur, &decoded_length));
  CHECK_AND_RETURN_FAILED(aws_byte_buf_init(out, allocator, decoded_length));
  CHECK_AND_RETURN_FAILED(aws_base64_decode(&cur, out));
  return AWS_OP_SUCCESS;
}

static int
s_rsa_encryption_roundtrip_helper(struct aws_allocator *allocator,
                                  struct aws_rsa_key_pair *key_pair,
                                  enum aws_rsa_encryption_algorithm algo) {

  struct aws_byte_cursor plaintext_cur;
  initialize_bigger_byte_cursor(&plaintext_cur);
  assume(aws_byte_cursor_is_valid(&plaintext_cur));

  /*make sure not to clobber anything in existing buffer*/
  struct aws_byte_cursor prefix;
  initialize_bigger_byte_cursor(&prefix);
  assume(aws_byte_cursor_is_valid(&prefix));

  struct aws_byte_buf ciphertext;
  CHECK_AND_RETURN_FAILED(
      aws_byte_buf_init(&ciphertext, allocator,
                        prefix.len + aws_rsa_key_pair_block_length(key_pair)));
  CHECK_AND_RETURN_FAILED(aws_byte_buf_append(&ciphertext, &prefix));
  CHECK_AND_RETURN_FAILED(
      aws_rsa_key_pair_encrypt(key_pair, algo, plaintext_cur, &ciphertext));

  struct aws_byte_cursor ciphertext_cur = aws_byte_cursor_from_buf(&ciphertext);
  if (!aws_byte_cursor_starts_with(&ciphertext_cur, &prefix)) {
    return AWS_OP_ERR;
  }

  aws_byte_cursor_advance(&ciphertext_cur, prefix.len);

  struct aws_byte_buf decrypted;
  CHECK_AND_RETURN_FAILED(
      aws_byte_buf_init(&decrypted, allocator,
                        prefix.len + aws_rsa_key_pair_block_length(key_pair)));
  CHECK_AND_RETURN_FAILED(aws_byte_buf_append(&decrypted, &prefix));
  CHECK_AND_RETURN_FAILED(
      aws_rsa_key_pair_decrypt(key_pair, algo, ciphertext_cur, &decrypted));

  struct aws_byte_cursor decrypted_cur = aws_byte_cursor_from_buf(&decrypted);
  if (!aws_byte_cursor_starts_with(&decrypted_cur, &prefix)) {
    return AWS_OP_ERR;
  }

  aws_byte_cursor_advance(&decrypted_cur, prefix.len);
  // TODO: check string equals
  // ASSERT_CURSOR_VALUE_CSTRING_EQUALS(decrypted_cur, TEST_ENCRYPTION_STRING);

  return AWS_OP_SUCCESS;
}

int rsa_encryption_roundtrip_from_user(struct aws_allocator *allocator,
                                       enum aws_rsa_encryption_algorithm algo) {

  struct aws_byte_cursor cur;
  initialize_bigger_byte_cursor(&cur);
  assume(aws_byte_cursor_is_valid(&cur));
  struct aws_byte_buf key_buf;
  CHECK_AND_RETURN_FAILED(s_byte_buf_decoded_from_base64_cur(
      allocator, cur, &key_buf));
  struct aws_rsa_key_pair *key_pair =
      aws_rsa_key_pair_new_from_private_key_pkcs1(
          allocator, aws_byte_cursor_from_buf(&key_buf));
  if (!key_pair) {
    return AWS_OP_ERR;
  }

  struct aws_byte_buf priv_key;
  CHECK_AND_RETURN_FAILED(aws_rsa_key_pair_get_private_key(
      key_pair, AWS_CAL_RSA_KEY_EXPORT_PKCS1, &priv_key));
  if (priv_key.len <= 0) {
    return AWS_OP_ERR;
  }

  CHECK_AND_RETURN_FAILED(
      s_rsa_encryption_roundtrip_helper(allocator, key_pair, algo));

  return AWS_OP_SUCCESS;
}

int rsa_verify_signing(struct aws_allocator *allocator) {
  struct aws_byte_cursor message;
  initialize_bigger_byte_cursor(&message);
  assume(aws_byte_cursor_is_valid(&message));

  struct aws_byte_cursor cur;
  initialize_bigger_byte_cursor(&cur);
  assume(aws_byte_cursor_is_valid(&cur));

  struct aws_byte_buf public_key_buf;
  CHECK_AND_RETURN_FAILED(s_byte_buf_decoded_from_base64_cur(
      allocator, cur, &public_key_buf));
  struct aws_rsa_key_pair *key_pair_public =
      aws_rsa_key_pair_new_from_public_key_pkcs1(
          allocator, aws_byte_cursor_from_buf(&public_key_buf));
  if (!key_pair_public) {
    return AWS_OP_ERR;
  }

  struct aws_byte_buf pub_key;
  CHECK_AND_RETURN_FAILED(aws_rsa_key_pair_get_public_key(
      key_pair_public, AWS_CAL_RSA_KEY_EXPORT_PKCS1, &pub_key));
  if (pub_key.len <= 0) {
    return AWS_OP_ERR;
  }

  uint8_t hash[AWS_SHA256_LEN];
  AWS_ZERO_ARRAY(hash);
  struct aws_byte_buf hash_value =
      aws_byte_buf_from_empty_array(hash, sizeof(hash));
  CHECK_AND_RETURN_FAILED(
      aws_sha256_compute(allocator, &message, &hash_value, 0));
  struct aws_byte_cursor hash_cur = aws_byte_cursor_from_buf(&hash_value);

  size_t str_len_3 = 0;
  struct aws_byte_cursor cur2;
  initialize_bigger_byte_cursor(&cur2);
  assume(aws_byte_cursor_is_valid(&cur2));

  struct aws_byte_buf signature_buf;
  CHECK_AND_RETURN_FAILED(s_byte_buf_decoded_from_base64_cur(
      allocator, cur2, &signature_buf));
  struct aws_byte_cursor signature_cur =
      aws_byte_cursor_from_buf(&signature_buf);

  CHECK_AND_RETURN_FAILED(aws_rsa_key_pair_verify_signature(
      key_pair_public, AWS_CAL_RSA_SIGNATURE_PKCS1_5_SHA256, hash_cur,
      signature_cur));

  return AWS_OP_SUCCESS;
}

int main() {
  /* data structure */
  struct aws_allocator *allocator = sea_allocator();

  enum aws_rsa_encryption_algorithm algo = nd_int();

  /* assume preconditions */
  assume(AWS_CAL_RSA_ENCRYPTION_PKCS1_5 <= algo);
  assume(algo <= AWS_CAL_RSA_ENCRYPTION_OAEP_SHA512);

  /* operation under verification */
  CHECK_AND_RETURN_FAILED(rsa_encryption_roundtrip_from_user(allocator, algo));

  CHECK_AND_RETURN_FAILED(rsa_verify_signing(allocator));

  /* assertions */

  return 0;
}