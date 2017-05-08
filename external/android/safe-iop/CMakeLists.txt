include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include)

# ashmem-host.c

set(SOURCES
    safe_iop.c
    ${CMAKE_SOURCE_DIR}/external/android/include/safe_iop.h)

add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
add_library(safe-iop STATIC ${SOURCES})
