include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include)

# ashmem-host.c

set(SOURCES
    android_reboot.c
    ashmem-dev.c
    atomic.c
    canned_fs_config.c
    config_utils.c
    debugger.c
    dlmalloc_stubs.c
    fs.c
    fs_config.c
    hashmap.c
    iosched_policy.c
    klog.c
    load_file.c
    multiuser.c
    native_handle.c
    open_memstream.c
    partition_utils.c
    process_name.c
    properties.c
    qtaguid.c
    record_stream.c
    sched_policy.c
    socket_inaddr_any_server_unix.c
    socket_local_client_unix.c
    socket_local_server_unix.c
    socket_loopback_client_unix.c
    socket_loopback_server_unix.c
    socket_network_client_unix.c
    strdup16to8.c
    strdup8to16.c
    strlcpy.c
    str_parms.c
    threads.c
    trace-host.c
    uevent.c
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/android_reboot.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/ashmem.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/atomic.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/bitops.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/compiler.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/config_utils.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/debugger.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/fs.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/hashmap.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/iosched_policy.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/jstring.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/klog.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/list.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/log.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/memory.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/misc.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/multiuser.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/native_handle.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/open_memstream.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/partition_utils.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/process_name.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/properties.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/qtaguid.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/record_stream.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/sched_policy.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/sockets.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/str_parms.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/threads.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/trace.h
    ${CMAKE_SOURCE_DIR}/external/android/include/cutils/uevent.h)

add_definitions(-D_GNU_SOURCE -D__ANDROID__)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
add_library(cutils STATIC ${SOURCES})
