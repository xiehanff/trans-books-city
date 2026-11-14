cmake_minimum_required(VERSION 3.16)

project(c_doc_test C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(CTest)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(HTTP_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/http")
set(HTTP_GENERATED_HEADER "${HTTP_GENERATED_DIR}/http_protocol.h")
set(HTTP_GENERATED_PROTOCOL_SOURCE "${HTTP_GENERATED_DIR}/http_protocol.c")
set(HTTP_GENERATED_SERVER_SOURCE "${HTTP_GENERATED_DIR}/http_server.c")
add_custom_command(
    OUTPUT
        ${HTTP_GENERATED_HEADER}
        ${HTTP_GENERATED_PROTOCOL_SOURCE}
        ${HTTP_GENERATED_SERVER_SOURCE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HTTP_GENERATED_DIR}
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_http_sources.py
            --source ${CMAKE_CURRENT_SOURCE_DIR}/../S09_简易HTTP协议.md
            --output-dir ${HTTP_GENERATED_DIR}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/../S09_简易HTTP协议.md
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_http_sources.py
    VERBATIM
)
add_custom_target(http_note_sources
    DEPENDS
        ${HTTP_GENERATED_HEADER}
        ${HTTP_GENERATED_PROTOCOL_SOURCE}
        ${HTTP_GENERATED_SERVER_SOURCE}
)

add_library(c_doc_examples STATIC
    lib/architecture_examples.c
    lib/basic_examples.c
    lib/bit_operations.c
    lib/control_flow.c
    lib/dynamic_array.c
    lib/function_pointer.c
    lib/generic_vector.c
    ${HTTP_GENERATED_PROTOCOL_SOURCE}
    lib/interface_examples.c
    lib/json_parser.c
    lib/linked_list.c
    lib/memory_pool.c
    lib/number_examples.c
    lib/sort_search.c
    lib/stack_queue.c
    lib/stdlib_examples.c
    lib/string_builder.c
    lib/string_examples.c
    lib/struct_array.c
)

add_dependencies(c_doc_examples http_note_sources)
target_include_directories(c_doc_examples PUBLIC lib ${HTTP_GENERATED_DIR})

add_executable(c_doc_tests
    test/test_architecture_examples.c
    test/test_basic_examples.c
    test/test_bit_operations.c
    test/test_control_flow.c
    test/test_dynamic_array.c
    test/test_function_pointer.c
    test/test_generic_vector.c
    test/test_http_protocol.c
    test/test_interface_examples.c
    test/test_json_parser.c
    test/test_linked_list.c
    test/test_main.c
    test/test_memory_pool.c
    test/test_number_examples.c
    test/test_sort_search.c
    test/test_stack_queue.c
    test/test_stdlib_examples.c
    test/test_string_builder.c
    test/test_string_examples.c
    test/test_struct_array.c
)

target_include_directories(c_doc_tests PRIVATE lib test ${HTTP_GENERATED_DIR})
target_link_libraries(c_doc_tests PRIVATE c_doc_examples)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(c_doc_examples PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(c_doc_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME architecture_examples_tests COMMAND c_doc_tests architecture_examples)
add_test(NAME basic_examples_tests COMMAND c_doc_tests basic_examples)
add_test(NAME bit_operations_tests COMMAND c_doc_tests bit_operations)
add_test(NAME control_flow_tests COMMAND c_doc_tests control_flow)
add_test(NAME dynamic_array_tests COMMAND c_doc_tests dynamic_array)
add_test(NAME function_pointer_tests COMMAND c_doc_tests function_pointer)
add_test(NAME generic_vector_tests COMMAND c_doc_tests generic_vector)
add_test(NAME http_protocol_tests COMMAND c_doc_tests http_protocol)
add_test(NAME interface_examples_tests COMMAND c_doc_tests interface_examples)
add_test(NAME json_parser_tests COMMAND c_doc_tests json_parser)
add_test(NAME linked_list_tests COMMAND c_doc_tests linked_list)
add_test(NAME memory_pool_tests COMMAND c_doc_tests memory_pool)
add_test(NAME number_examples_tests COMMAND c_doc_tests number_examples)
add_test(NAME sort_search_tests COMMAND c_doc_tests sort_search)
add_test(NAME stack_queue_tests COMMAND c_doc_tests stack_queue)
add_test(NAME stdlib_examples_tests COMMAND c_doc_tests stdlib_examples)
add_test(NAME string_builder_tests COMMAND c_doc_tests string_builder)
add_test(NAME string_examples_tests COMMAND c_doc_tests string_examples)
add_test(NAME struct_array_tests COMMAND c_doc_tests struct_array)
add_test(
    NAME markdown_manifest_coverage
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_manifest.py
            --check
)
add_test(
    NAME markdown_compile_fail_examples
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_manifest.py
            --compile-fail
            --compiler ${CMAKE_C_COMPILER}
)
add_test(
    NAME platform_examples
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_platform_examples.py
            --check
)
set_tests_properties(
    markdown_compile_fail_examples
    markdown_manifest_coverage
    platform_examples
    PROPERTIES TIMEOUT 30
)
