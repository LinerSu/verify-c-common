/*
 * DER (Distinguished Encoding Rules) is a binary encoding format for 
 * ASN.1 (Abstract Syntax Notation One) data structures. It provides unique
 * encoding of each inidividual value. E.g. an Integer value would be encoded 
 * with a byte indicating the type (int), followed by a byte or bytes indicating 
 * the length of the integer, and then the actual integer value.
 */

#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>

#include <seahorn/seahorn.h>
#include <aws/cal/cal.h>
#include <aws/cal/private/der.h>

#define CHECK_AND_RETURN_FAILED(COND) \
    if (COND != AWS_OP_SUCCESS) { \
        return -1; \
    }


extern NONDET_FN_ATTR int64_t nd_int64_t(void);
#define MAX_ITERATION_BOUND 10

static int warpper_encoding(struct aws_der_encoder *encoder) {
    size_t length = nd_size_t();
    assume(length > 0); assume(length < 5);
    uint8_t *array = bounded_malloc_havoc(length);

    struct aws_byte_cursor cur1 = aws_byte_cursor_from_array(array, length);
    struct aws_byte_cursor cur2 = aws_byte_cursor_from_array(array, length);
    struct aws_byte_cursor cur3 = aws_byte_cursor_from_array(array, length);
    CHECK_AND_RETURN_FAILED(aws_der_encoder_write_boolean(encoder, nd_bool()));
    CHECK_AND_RETURN_FAILED(aws_der_encoder_write_unsigned_integer(encoder, cur1));
    CHECK_AND_RETURN_FAILED(aws_der_encoder_write_null(encoder));
    CHECK_AND_RETURN_FAILED(aws_der_encoder_write_bit_string(encoder, cur2));
    CHECK_AND_RETURN_FAILED(aws_der_encoder_write_octet_string(encoder, cur3));
    return AWS_OP_SUCCESS;
}

static int warpper_decoding(struct aws_der_decoder *decoder) {
    while (aws_der_decoder_next(decoder)) {
      enum aws_der_type ty = aws_der_decoder_tlv_type(decoder);
      size_t len = aws_der_decoder_tlv_length(decoder);
      struct aws_byte_cursor *decoded = malloc(sizeof(struct aws_byte_cursor));
      switch(ty) {
        case AWS_DER_INTEGER:
          CHECK_AND_RETURN_FAILED(aws_der_decoder_tlv_unsigned_integer(decoder, decoded));
          break;
        case AWS_DER_BOOLEAN: {
          bool flag = nd_bool();
          CHECK_AND_RETURN_FAILED(aws_der_decoder_tlv_boolean(decoder, &flag));
        }
          break;
        case AWS_DER_BIT_STRING:
        case AWS_DER_OCTET_STRING:
          CHECK_AND_RETURN_FAILED(aws_der_decoder_tlv_string(decoder, decoded));
          break;
        case AWS_DER_NULL:
          break;
        default:
          break;
      }
    }
    return AWS_OP_SUCCESS;
}

int main() {
    /* data structure */
    struct aws_allocator *allocator = sea_allocator();
    /* assume preconditions */

    /* operation under verification */
    struct aws_der_encoder *encoder = aws_der_encoder_new(allocator, MAX_SIZE);
    if (!encoder) {
      return -1;
    }
    // create a set encoder
    CHECK_AND_RETURN_FAILED(aws_der_encoder_begin_set(encoder));
    CHECK_AND_RETURN_FAILED(warpper_encoding(encoder));
    CHECK_AND_RETURN_FAILED(aws_der_encoder_end_set(encoder));
    struct aws_byte_cursor *encoded = bounded_malloc_havoc(sizeof(struct aws_byte_cursor));
    CHECK_AND_RETURN_FAILED(aws_der_encoder_get_contents(encoder, encoded));
    struct aws_der_decoder *decoder = aws_der_decoder_new(allocator, *encoded);
    if (!decoder) {
      return -1;
    }
    CHECK_AND_RETURN_FAILED(warpper_decoding(decoder));
    // create a sequence encoder
    // CHECK_AND_RETURN_FAILED(aws_der_encoder_begin_sequence(encoder));
    // CHECK_AND_RETURN_FAILED(warpper_encoding(encoder));
    // CHECK_AND_RETURN_FAILED(aws_der_encoder_end_sequence(encoder));
    // CHECK_AND_RETURN_FAILED(aws_der_encoder_get_contents(encoder, encoded));
    // decoder = aws_der_decoder_new(allocator, *encoded);
    // if (!decoder) {
    //   return -1;
    // }
    // CHECK_AND_RETURN_FAILED(warpper_decoding(decoder));
    aws_der_decoder_destroy(decoder);
    aws_der_encoder_destroy(encoder);
    /* assertions */

    return 0;
}