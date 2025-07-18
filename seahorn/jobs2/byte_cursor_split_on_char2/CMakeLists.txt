add_executable(byte_cursor_split_on_char_n2
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  aws_byte_cursor_split_on_char_n_harness2.c)
sea_attach_bc_link(byte_cursor_split_on_char_n2)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(byte_cursor_split_on_char_n2)