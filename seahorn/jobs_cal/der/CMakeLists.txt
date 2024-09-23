add_executable(der
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_CAL_ROOT}/source/der.c
  aws_der_harness.c)
set(MAX_SIZE ${MAX_BUFFER_SIZE})
target_compile_definitions(der PUBLIC MAX_SIZE=${MAX_SIZE})
sea_attach_bc_link(der)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(der)