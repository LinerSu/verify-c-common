add_executable(input_stream_from_cursor
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_COMMON_ROOT}/source/ref_count.c
  ${AWS_C_IO_ROOT}/source/stream.c
  aws_input_stream_from_cursor_harness.c)
sea_overlink_libraries(input_stream_from_cursor aws_ref_count_overrides.ir)
sea_attach_bc_link(input_stream_from_cursor)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(input_stream_from_cursor)