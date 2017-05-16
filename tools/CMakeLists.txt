include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external/android/include)

add_executable(demo_service demo_service.cpp)
target_link_libraries(demo_service binder)

add_executable(throughput_test throughput_test.cpp)
target_link_libraries(throughput_test binder)

add_executable(binder_lib_test binder_lib_test.cpp)
target_link_libraries(binder_lib_test
    binder
    ${GMOCK_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES})
