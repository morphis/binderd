include_directories(
    ${CMAKE_SOURCE_DIR}/external/android/include)

set(SOURCES
    config_write.c
    event_tag_map.c
    fake_log_device.c
    fake_writer.c
    log_event_list.c
    log_event_write.c
    logger_lock.c
    logger_name.c
    logger_write.c
    event.logtags
    ${CMAKE_SOURCE_DIR}/external/android/include/log/event_tag_map.h
    ${CMAKE_SOURCE_DIR}/external/android/include/log/uio.h
    ${CMAKE_SOURCE_DIR}/external/android/include/log/logd.h
    ${CMAKE_SOURCE_DIR}/external/android/include/log/log_read.h
    ${CMAKE_SOURCE_DIR}/external/android/include/log/logprint.h
    ${CMAKE_SOURCE_DIR}/external/android/include/log/log.h
    ${CMAKE_SOURCE_DIR}/external/android/include/log/logger.h)

add_definitions(-D_GNU_SOURCE -D__ANDROID__ -DFAKE_LOG_DEVICE=1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
add_library(log STATIC ${SOURCES})
