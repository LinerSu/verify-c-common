add_executable(pem_objects_init_from_file_contents
  ${AWS_C_COMMON_ROOT}/source/array_list.c
  ${AWS_C_COMMON_ROOT}/source/byte_buf.c
  ${AWS_C_COMMON_ROOT}/source/string.c
  ${AWS_C_COMMON_ROOT}/source/common.c
  ${AWS_C_COMMON_ROOT}/source/encoding.c
  ${AWS_C_IO_ROOT}/source/pem.c
  aws_pem_objects_init_from_file_contents_harness.c)
sea_attach_bc_link(pem_objects_init_from_file_contents)
configure_file(sea.yaml sea.yaml @ONLY)
sea_add_unsat_test(pem_objects_init_from_file_contents)