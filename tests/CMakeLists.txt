include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external/android/include
  ${CMAKE_SOURCE_DIR}/external/asio/include
)

set(TEST_COMMON_SOURCE
  common/statistics.cpp
  common/benchmark.cpp
  common/standalone_server.cpp)

add_library(binderd-test-common ${TEST_COMMON_SOURCE})
target_link_libraries(binderd-test-common binderd-core)

macro(BINDERD_ADD_TEST test_name src)
  add_executable(
    ${test_name}
    ${src}
  )

  target_link_libraries(
    ${test_name}

    process-cpp

    binder

    binderd-core
    binderd-test-common

    ${GMOCK_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES}

    ${ARGN}

    ${CMAKE_THREAD_LIBS_INIT}
  )

  add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name} --gtest_filter=*-*requires*)
endmacro(BINDERD_ADD_TEST)

add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(acceptance)
