add_executable(rsa
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_COMMON_ROOT}/source/encoding.c
  ${AWS_C_CAL_ROOT}/source/der.c
  ${AWS_C_CAL_ROOT}/source/rsa.c
  ${AWS_C_CAL_ROOT}/source/unix/openssl_rsa.c
  ${AWS_C_CAL_ROOT}/source/hash.c
  ${AWS_C_CAL_ROOT}/source/unix/opensslcrypto_hash.c
  aws_rsa_harness.c)
set(MAX_SIZE ${MAX_BUFFER_SIZE})
target_compile_definitions(rsa PUBLIC MAX_SIZE=${MAX_SIZE})
# sea_overlink_libraries(rsa aws_ref_count_overrides.ir)
sea_attach_bc_link(rsa)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(rsa)