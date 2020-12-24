add_executable(byte_buf_from_array
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  aws_byte_buf_from_array_harness.c)
target_compile_definitions(byte_buf_from_array
  PUBLIC MAX_INITIAL_SIZE=${MAX_INITIAL_SIZE})
sea_attach_bc_link(byte_buf_from_array)
sea_add_unsat_test(byte_buf_from_array)

# klee
sea_add_klee(byte_buf_from_array ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  aws_byte_buf_from_array_harness.c)
