include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include)

set(SOURCES system_properties.cpp)

add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
add_library(system-properties STATIC ${SOURCES})
