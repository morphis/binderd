include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/external/android/include
)

macro(BINDERD_ADD_TEST test_name src)
  add_executable(
    ${test_name}
    ${src}
  )

  target_link_libraries(
    ${test_name}

    binder

    ${GMOCK_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES}

    ${ARGN}

    ${CMAKE_THREAD_LIBS_INIT}
  )

  add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name} --gtest_filter=*-*requires*)
endmacro(BINDERD_ADD_TEST)

add_subdirectory(integration)
