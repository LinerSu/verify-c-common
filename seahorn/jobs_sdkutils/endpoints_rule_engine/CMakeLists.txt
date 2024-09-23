add_executable(endpoints_rule_engine
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_COMMON_ROOT}/source/ref_count.c
  ${AWS_C_SDKUTILS_ROOT}/source/endpoints_ruleset.c
  ${AWS_C_SDKUTILS_ROOT}/source/endpoints_rule_engine.c
  ${AWS_C_SDKUTILS_ROOT}/source/endpoints_types_impl.c
  ${AWS_C_SDKUTILS_ROOT}/source/endpoints_util.c
  aws_endpoints_rule_engine_harness.c)
set(MAX_TABLE_SIZE 8)
target_compile_definitions(endpoints_rule_engine
                            PUBLIC
                            MAX_TABLE_SIZE=${MAX_TABLE_SIZE})
sea_link_libraries(endpoints_rule_engine hash_table.opt.ir)
sea_overlink_libraries(endpoints_rule_engine aws_hash_string_overrides.ir)
sea_overlink_libraries(endpoints_rule_engine update_template_size_override.ir)
sea_overlink_libraries(endpoints_rule_engine hash_table_state_is_valid_override.ir)
sea_overlink_libraries(endpoints_rule_engine aws_hash_byte_cursor_ptr_overrides.ir)
sea_attach_bc_link(endpoints_rule_engine)
sea_add_unsat_test(endpoints_rule_engine)