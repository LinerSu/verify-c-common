add_executable(huffman_test_transitive
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMPRESSION_ROOT}/source/huffman.c
  ${AWS_C_COMPRESSION_ROOT}/source/huffman_testing.c
  aws_huffman_test_transitive_harness.c)
set(MAX_SIZE ${MAX_STRING_LEN})
target_compile_definitions(huffman_test_transitive PUBLIC MAX_SIZE=${MAX_SIZE})
sea_overlink_libraries(huffman_test_transitive byte_cursor_advance_nospec_override.ir)
sea_overlink_libraries(huffman_test_transitive aws_nospec_mask_overrides.ir)
sea_attach_bc_link(huffman_test_transitive)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(huffman_test_transitive)