add_executable(sha256_hmac
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_CAL_ROOT}/source/der.c
  ${AWS_C_CAL_ROOT}/source/hmac.c
  ${AWS_C_CAL_ROOT}/source/unix/opensslcrypto_hmac.c
  aws_sha256_hmac_harness.c)
set(MAX_SIZE ${MAX_BUFFER_SIZE})
target_compile_definitions(sha256_hmac PUBLIC MAX_SIZE=${MAX_SIZE})
# sea_overlink_libraries(sha256_hmac aws_ref_count_overrides.ir)
sea_attach_bc_link(sha256_hmac)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(sha256_hmac)