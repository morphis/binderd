include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/asio/include
    ${CMAKE_SOURCE_DIR}/external/process-cpp-minimal/include/
    ${CMAKE_SOURCE_DIR}/external/android/include/)

set(CORE_SOURCES
    binderd/common/optional.h
    binderd/common/do_not_copy_or_move.h
    binderd/common/utils.cpp
    binderd/common/variable_length_array.h
    binderd/common/binary_writer.cpp
    binderd/common/binary_reader.cpp
    binderd/common/ref_counted.cpp

    binderd/cli.cpp
    binderd/logger.cpp

    binderd/constants.h
    binderd/macros.h
    binderd/binder_api.h
    binderd/sessions.h
    binderd/server_session.cpp
    binderd/messenger.h
    binderd/socket_messenger.cpp
    binderd/registry.cpp
    binderd/buffer.cpp
    binderd/message.cpp
    binderd/transaction_data.cpp
    binderd/transaction_status.cpp
    binderd/transaction_data_from_message.cpp
    binderd/writable_transaction_data.cpp
    binderd/parcel_transaction_data_writer.cpp
    binderd/parcel_transaction_data_reader.cpp
    binderd/status.h
    binderd/message_parser.cpp
    binderd/server.cpp
    binderd/server_impl.cpp
    binderd/client.cpp
    binderd/client_impl.cpp
    binderd/object.h
    binderd/local_object.cpp
    binderd/remote_object.cpp
    binderd/object_pool.cpp
    binderd/object_translator.cpp

    binderd/service_manager_base.cpp
    binderd/service_manager_service.cpp
    binderd/service_manager_proxy.cpp)

add_library(binderd-core STATIC ${CORE_SOURCES})
target_link_libraries(binderd-core
    pthread
    ${Boost_LDFLAGS}
    ${Boost_LIBRARIES})

set(BINDERD_SOURCES
    main.cpp
    binderd/cmds/server.cpp
    binderd/cmds/client.cpp
    binderd/cmds/service_manager.cpp
    binderd/cmds/service.cpp
    binderd/cmds/monitor.cpp
    binderd/daemon.cpp)

add_executable(binderd ${BINDERD_SOURCES})
target_link_libraries(binderd binderd-core)
