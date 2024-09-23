/*
 * event loop is used to manage communication between different processes
 * or threads. It waits an event and handle event to dispatch message.
 */

#include <aws/common/atomics.h>
#include <aws/common/clock.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/common/system_info.h>
#include <aws/common/task_scheduler.h>
#include <aws/common/thread.h>
#include <aws/io/event_loop.h>
#include <byte_buf_helper.h>
#include <config.h>
#include <nondet.h>
#include <proof_allocators.h>
#include <seahorn/seahorn.h>
#include <string_helper.h>
#include <utils.h>

extern NONDET_FN_ATTR int64_t nd_int64_t(void);
#define MAX_ITERATION_BOUND 10
struct state;
typedef int(thread_state_fn)(struct state *stat);

#define CHECK_AND_RETURN_FAILED(COND)                                          \
  if (COND != AWS_OP_SUCCESS) {                                                \
    return AWS_OP_ERR;                                                         \
  }

#define MAX_STATE_FUN_NUM 2

int aws_open_nonblocking_posix_pipe(int pipe_fds[2]) {
  // call pipe for creating undirectional data channel
  if (nd_bool()) {
    return 0;
  } else {
    return -1;
  }
}

int aws_high_res_clock_get_ticks(uint64_t *timestamp) {
  if (nd_bool()) {
    return AWS_OP_ERR;
  }
  *timestamp = nd_uint64_t();
  return AWS_OP_SUCCESS;
}

struct state {
  struct aws_allocator *alloc;
  struct aws_event_loop *event_loop;

  bool done;
  int error_code;
  struct aws_mutex mutex;
  struct aws_condition_variable condition_variable;

  thread_state_fn **state_functions;
  size_t current_state;
  size_t last_printed_state;

  /* data for tests */
  struct aws_io_handle read_handle;
  struct aws_io_handle write_handle;
  int read_handle_event_counts[AWS_IO_EVENT_TYPE_ERROR + 1];
  int write_handle_event_counts[AWS_IO_EVENT_TYPE_ERROR + 1];

  enum { TIMER_NOT_SET, TIMER_WAITING, TIMER_DONE } timer_state;
  struct aws_task timer_task;
};

int simple_pipe_open(struct aws_io_handle *read_handle,
                     struct aws_io_handle *write_handle) {
  AWS_ZERO_STRUCT(*read_handle);
  AWS_ZERO_STRUCT(*write_handle);

  int pipe_fds[2];
  CHECK_AND_RETURN_FAILED(aws_open_nonblocking_posix_pipe(pipe_fds));

  read_handle->data.fd = pipe_fds[0];
  write_handle->data.fd = pipe_fds[1];

  return AWS_OP_SUCCESS;
}

void simple_pipe_close(struct aws_io_handle *read_handle,
                       struct aws_io_handle *write_handle) {
  // close IO, treat as noop
  return;
}


static void s_thread_abort(struct state *stat) {
  aws_mutex_lock(&stat->mutex);
  stat->error_code = AWS_OP_ERR;
  stat->done = true;
  aws_condition_variable_notify_one(&stat->condition_variable);
  aws_mutex_unlock(&stat->mutex);
}

static void s_thread_update(struct state *stat) {
  thread_state_fn *current_fn;
  size_t current_state = stat->current_state;
  for (unsigned i = current_state; i < MAX_STATE_FUN_NUM; ++i) {
    current_fn = stat->state_functions[i];

    if (!current_fn) {
      /* We've reached the final state, success */
      aws_mutex_lock(&stat->mutex);
      stat->error_code = AWS_OP_SUCCESS;
      stat->done = true;
      aws_condition_variable_notify_one(&stat->condition_variable);
      aws_mutex_unlock(&stat->mutex);
      return;
    }

    int err = current_fn(stat);

    if (err == AWS_OP_SUCCESS) {
      /* Go to next state, loop again */
      stat->current_state++;

    } else if (err == -2) {
      /* End loop, wait for update function to be invoked again */
      return;

    } else /* AWS_OP_ERR */ {
      /* End loop, end it all */
      s_thread_abort(stat);
      return;
    }
  }
}

/* Count how many times each type of event fires on the readable and writable handles */
static void s_io_event_counter(
    struct aws_event_loop *event_loop,
    struct aws_io_handle *handle,
    int events,
    void *user_data) {

    (void)event_loop;
    (void)handle;
    struct state *stat = user_data;

    int *event_counts;
    if (handle == &stat->read_handle) {
        event_counts = stat->read_handle_event_counts;

    } else if (handle == &stat->write_handle) {
        event_counts = stat->write_handle_event_counts;

    } else {
        return s_thread_abort(stat);
    }

    for (int flag = 1; flag <= AWS_IO_EVENT_TYPE_ERROR; flag <<= 1) {
        if (events & flag) {
            event_counts[flag] += 1;
        }
    }

    s_thread_update(stat);
}

static bool s_thread_pred(void *arg) {
    struct state *stat = arg;
    return stat->done;
}

static int s_state_subscribe(struct state *stat) {
  CHECK_AND_RETURN_FAILED(aws_event_loop_subscribe_to_io_events(
      stat->event_loop, &stat->read_handle, AWS_IO_EVENT_TYPE_READABLE,
      s_io_event_counter, stat));

  CHECK_AND_RETURN_FAILED(aws_event_loop_subscribe_to_io_events(
      stat->event_loop, &stat->write_handle, AWS_IO_EVENT_TYPE_WRITABLE,
      s_io_event_counter, stat));

  return AWS_OP_SUCCESS;
}

static int s_state_unsubscribe(struct state *stat) {
  CHECK_AND_RETURN_FAILED(aws_event_loop_unsubscribe_from_io_events(
      stat->event_loop, &stat->read_handle));
  CHECK_AND_RETURN_FAILED(aws_event_loop_unsubscribe_from_io_events(
      stat->event_loop, &stat->write_handle));
  return AWS_OP_SUCCESS;
}

static void s_thread_update_task(struct aws_task *task, void *arg,
                                 enum aws_task_status status) {
  (void)task;
  struct state *stat = arg;

  if (status != AWS_TASK_STATUS_RUN_READY) {
    return s_thread_abort(stat);
  }

  s_thread_update(stat);
}

static void s_timer_done_task(struct aws_task *task, void *arg,
                              enum aws_task_status status) {
  (void)task;
  struct state *stat = arg;

  if (status != AWS_TASK_STATUS_RUN_READY) {
    return s_thread_abort(stat);
  }

  stat->timer_state = TIMER_DONE;
  s_thread_update(stat);
}

static int s_thread_run(struct aws_allocator *alloc,
                        thread_state_fn *state_functions[]) {

  /* Set up task running state */
  // struct state stat = {
  //     .alloc = alloc,
  //     .event_loop =
  //         aws_event_loop_new_default(alloc, aws_high_res_clock_get_ticks),
  //     .mutex = AWS_MUTEX_INIT,
  //     .condition_variable = AWS_CONDITION_VARIABLE_INIT,
  //     .state_functions = state_functions,
  //     .last_printed_state = -1,
  // };
  struct aws_event_loop *event_loop = aws_event_loop_new_default(alloc, aws_high_res_clock_get_ticks);

  // if (!stat.event_loop) {
  //   return AWS_OP_ERR;
  // }
  return AWS_OP_SUCCESS;
  // CHECK_AND_RETURN_FAILED(aws_event_loop_run(stat.event_loop));

  /* Set up data to test with */
  // CHECK_AND_RETURN_FAILED(
  //     simple_pipe_open(&stat.read_handle, &stat.write_handle));
  // aws_task_init(&stat.timer_task, s_timer_done_task, &stat, "timer_done");

  // /* Wait for stat to finish running its state functions on the event-loop
  //  * thread */
  // CHECK_AND_RETURN_FAILED(aws_mutex_lock(&stat.mutex));
  // struct aws_task task;
  // aws_task_init(&task, s_thread_update_task, &stat,
  //                                       "thread_stat_update");
  // // aws_event_loop_schedule_task_now(stat.event_loop, &task);
  // CHECK_AND_RETURN_FAILED(aws_condition_variable_wait_pred(
  //     &stat.condition_variable, &stat.mutex, s_thread_pred, &stat));
  // CHECK_AND_RETURN_FAILED(aws_mutex_unlock(&stat.mutex));

  // /* Clean up stat*/
  // // aws_event_loop_destroy(stat.event_loop);

  // /* Clean up data */
  // simple_pipe_close(&stat.read_handle, &stat.write_handle);

  // /* Return stat results */
  // return stat.error_code;
}

int main() {
  /* data structure */
  struct aws_allocator *allocator = sea_allocator();
  thread_state_fn *state_functions[] = {s_state_subscribe, s_state_unsubscribe};
  /* assume preconditions */
  /* operation under verification */
  CHECK_AND_RETURN_FAILED(s_thread_run(allocator, state_functions));

  /* assertions */

  return 0;
}