/*
 *
 */

#include <seahorn/seahorn.h>
#include <aws/compression/huffman.h>
#include <aws/compression/private/huffman_testing.h>
#include <utils.h>
#include <proof_allocators.h>
#include <nondet.h>
#include <config.h>
#include <string_helper.h>
#define STEP_SZ 2

static uint8_t count_bits(uint32_t value) {
    uint8_t count = 0;
    while (value) {
        count++;
        value >>= 1; // Shift the value right by 1 bit
    }
    return count;
}

static struct aws_huffman_code encode_symbol(uint8_t symbol, void *userdata) {
    (void)userdata;
    struct aws_huffman_code rev;
    // specs: pattern: 0x32, num_bits matches the exact bits used by pattern
    rev.pattern = 0x32;
    rev.num_bits = 5;
    return rev;
}

static uint8_t decode_symbol(uint32_t bits, uint8_t *symbol, void *userdata) {
    (void)userdata;
    uint8_t ret = 5;
    return ret;
}

struct aws_huffman_symbol_coder *sea_get_coder(void) {

    static struct aws_huffman_symbol_coder coder = {
        .encode = encode_symbol,
        .decode = decode_symbol,
        .userdata = NULL,
    };
    return &coder;
}

const char *nd_c_str(size_t *len) {
  size_t alloc_size;
  alloc_size = nd_size_t();
  // allocate no more than sea_max_string_len() + 1
  assume(alloc_size > 0);
  assume(alloc_size <= MAX_SIZE);

  // this allocation never fails
  char *str = bounded_malloc_havoc(alloc_size);
  size_t str_len = alloc_size - 1;
  size_t idx = nd_size_t();
  assume(idx < str_len);
  char c = nd_char();
  assume(c != '\0');
  str[idx] = c;
  str[str_len] = '\0';
  *len = str_len;
  return str;
}

int main() {
    /* data structure */
    size_t str_len = 0;
    const char *inputs = nd_c_str(&str_len);
    if (!inputs) {
        return AWS_OP_ERR;
    }

    size_t encoded_sz = nd_size_t();
    assume(encoded_sz <= str_len);
    const char *error_message = NULL;
    /* assume preconditions */
    size_t step_sizes[STEP_SZ];
    for (size_t i = 0; i < STEP_SZ; i++)
    {
        step_sizes[i] = 1 << i;
    }

    /* perform operation under verification */
    for (size_t i = 0; i < STEP_SZ; ++i) {
        const size_t step_size = step_sizes[i];
        const char *error_message = NULL;
        huffman_test_transitive_chunked(sea_get_coder(), inputs, str_len, encoded_sz, step_size, &error_message);
    }

    /* assertions */

    return 0;
}
