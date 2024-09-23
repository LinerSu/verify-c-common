add_executable(sanitize_pem
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_IO_ROOT}/source/pem.c
  aws_sanitize_pem_harness.c)
if(MAX_BUFFER_SIZE GREATER 26)
  set(MAX_SIZE ${MAX_BUFFER_SIZE})
else()
  set(MAX_SIZE 30)
endif()
target_compile_definitions(sanitize_pem PUBLIC MAX_SIZE=${MAX_SIZE})
sea_attach_bc_link(sanitize_pem)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(sanitize_pem)