
#include <aws/common/common.h>
#include <aws/common/math.h>

/**
 * Function to find the smallest result that is power of 2 >= n. Returns AWS_OP_ERR if this cannot
 * be done without overflow
 */
AWS_STATIC_IMPL int aws_round_up_to_power_of_two(size_t n, size_t *result) {
    if (n == 0) {
        *result = 1;
        return AWS_OP_SUCCESS;
    }
    if (n > SIZE_MAX_POWER_OF_TWO) {
        return aws_raise_error(AWS_ERROR_OVERFLOW_DETECTED);
    }

    size_t power = 1;
    while (power < n)
    {
        if (power > SIZE_MAX_POWER_OF_TWO) {
            return aws_raise_error(AWS_ERROR_OVERFLOW_DETECTED);
        }
        power *= 2;
    }

    *result = power;
    return AWS_OP_SUCCESS;
}