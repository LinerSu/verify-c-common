add_executable(profile_collection_new_from_buffer
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_COMMON_ROOT}/source/ref_count.c
  ${AWS_C_SDKUTILS_ROOT}/source/aws_profile.c
  aws_profile_collection_new_from_buffer_harness.c)
set(MAX_TABLE_SIZE 8)
target_compile_definitions(profile_collection_new_from_buffer
                            PUBLIC
                            MAX_TABLE_SIZE=${MAX_TABLE_SIZE})
sea_link_libraries(profile_collection_new_from_buffer hash_table.opt.ir)
sea_overlink_libraries(profile_collection_new_from_buffer aws_hash_string_overrides.ir)
sea_overlink_libraries(profile_collection_new_from_buffer update_template_size_override.ir)
sea_overlink_libraries(profile_collection_new_from_buffer hash_table_state_is_valid_override.ir)
sea_overlink_libraries(profile_collection_new_from_buffer byte_cursor_advance_nospec_override.ir)
sea_attach_bc_link(profile_collection_new_from_buffer)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(profile_collection_new_from_buffer)