include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include
    ${CMAKE_SOURCE_DIR}/src)

set(SOURCES
    binder.cpp
    service_manager.cpp)

add_definitions(-D_GNU_SOURCE -D__ANDROID__)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-permissive")
add_library(servicemanager SHARED ${SOURCES})
target_link_libraries(servicemanager binder binderd-core pthread)
