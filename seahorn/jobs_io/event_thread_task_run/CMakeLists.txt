add_executable(event_thread_task_run
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_COMMON_ROOT}/source/ref_count.c
  ${AWS_C_COMMON_ROOT}/source/condition_variable.c
  ${AWS_C_COMMON_ROOT}/source/posix/thread.c
  ${AWS_C_COMMON_ROOT}/source/posix/mutex.c
  ${AWS_C_COMMON_ROOT}/source/posix/system_info.c
  ${AWS_C_COMMON_ROOT}/source/posix/cond_variable.c
  ${AWS_C_IO_ROOT}/source/event_loop.c
  ${AWS_C_IO_ROOT}/source/linux/epoll_event_loop.c
  aws_event_thread_task_run_harness.c)
set(MAX_TABLE_SIZE 8)
target_compile_definitions(event_thread_task_run
                            PUBLIC
                            MAX_TABLE_SIZE=${MAX_TABLE_SIZE})
sea_link_libraries(event_thread_task_run hash_table.opt.ir)
sea_overlink_libraries(event_thread_task_run aws_hash_string_overrides.ir)
sea_overlink_libraries(event_thread_task_run update_template_size_override.ir)
sea_overlink_libraries(event_thread_task_run hash_table_state_is_valid_override.ir)
sea_overlink_libraries(event_thread_task_run aws_hash_byte_cursor_ptr_overrides.ir)
sea_overlink_libraries(event_thread_task_run aws_ref_count_overrides.ir)
sea_overlink_libraries(event_thread_task_run snprintf_overrides.ir)
sea_attach_bc_link(event_thread_task_run)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(event_thread_task_run)