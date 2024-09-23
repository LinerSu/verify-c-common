add_executable(ecc
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_CAL_ROOT}/source/der.c
  ${AWS_C_CAL_ROOT}/source/ecc.c
  ${AWS_C_CAL_ROOT}/source/unix/opensslcrypto_ecc.c
  aws_ecc_harness.c)
set(MAX_SIZE ${MAX_BUFFER_SIZE})
target_compile_definitions(ecc PUBLIC MAX_SIZE=${MAX_SIZE})
sea_overlink_libraries(ecc aws_ref_count_overrides.ir)
sea_overlink_libraries(ecc byte_cursor_advance_nospec_override.ir)
sea_overlink_libraries(ecc aws_nospec_mask_overrides.ir)
sea_attach_bc_link(ecc)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(ecc)