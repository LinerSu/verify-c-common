/*
 * event loop is used to manage communication between different processes 
 * or threads. It waits an event and handle event to dispatch message.
 */

#include <seahorn/seahorn.h>
#include <aws/io/event_loop.h>
#include <utils.h>
#include <proof_allocators.h>
#include <string_helper.h>
#include <byte_buf_helper.h>
#include <nondet.h>
#include <config.h>

extern NONDET_FN_ATTR int64_t nd_int64_t(void);
#define MAX_ITERATION_BOUND 10

int main() {
    /* data structure */
    struct aws_allocator *allocator = sea_allocator();
    /* assume preconditions */

    /* operation under verification */
    struct aws_event_loop_group *event_loop_group = aws_event_loop_group_new_default(allocator, 1, NULL);

    if (!event_loop_group) {
        return -1;
    }

    size_t el_count = aws_event_loop_group_get_loop_count(event_loop_group);

    // struct aws_event_loop *event_loop = aws_event_loop_group_get_next_loop(event_loop_group);
    // sassert(event_loop != NULL);

    // aws_event_loop_group_release(event_loop_group);
    /* assertions */

    return 0;
}