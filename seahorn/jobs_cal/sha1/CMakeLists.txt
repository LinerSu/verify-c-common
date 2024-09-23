add_executable(sha1
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_CAL_ROOT}/source/der.c
  ${AWS_C_CAL_ROOT}/source/hash.c
  ${AWS_C_CAL_ROOT}/source/unix/opensslcrypto_hash.c
  aws_sha1_harness.c)
set(MAX_SIZE ${MAX_BUFFER_SIZE})
target_compile_definitions(sha1 PUBLIC MAX_SIZE=${MAX_SIZE})
# sea_overlink_libraries(sha1 aws_ref_count_overrides.ir)
sea_attach_bc_link(sha1)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(sha1)