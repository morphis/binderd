include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include)

set(SOURCES
    binder.c
    service_manager.c)

add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
add_executable(servicemanager ${SOURCES})
target_link_libraries(servicemanager binder pthread)
