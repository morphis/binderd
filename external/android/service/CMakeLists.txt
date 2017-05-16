include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include)

set(SOURCES service.cpp)

add_definitions(-D_GNU_SOURCE -D__ANDROID__)
add_executable(service ${SOURCES})
target_link_libraries(service binder pthread)
