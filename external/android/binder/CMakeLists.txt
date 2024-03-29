include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/android/include)

set(SOURCES
    AppOpsManager.cpp
    Binder.cpp
    BpBinder.cpp
    BufferedTextOutput.cpp
    Debug.cpp
    IAppOpsCallback.cpp
    IAppOpsService.cpp
    IBatteryStats.cpp
    IInterface.cpp
    IMediaResourceMonitor.cpp
    IMemory.cpp
    IPCThreadState.cpp
    IPermissionController.cpp
    IProcessInfoService.cpp
    IResultReceiver.cpp
    IServiceManager.cpp
    MemoryBase.cpp
    MemoryDealer.cpp
    MemoryHeapBase.cpp
    Parcel.cpp
    TransactionDataFromParcel.cpp
    PermissionCache.cpp
    PersistableBundle.cpp
    ProcessInfoService.cpp
    ProcessState.cpp
    Static.cpp
    Status.cpp
    TextOutput.cpp
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/AppOpsManager.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/Binder.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/BinderService.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/BpBinder.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/BufferedTextOutput.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/Debug.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IAppOpsCallback.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IAppOpsService.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IBatteryStats.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IBinder.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IInterface.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IMediaResourceMonitor.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IMemory.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IPCThreadState.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IPermissionController.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IProcessInfoService.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IResultReceiver.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/IServiceManager.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/MemoryBase.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/MemoryDealer.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/MemoryHeapBase.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/Parcelable.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/Parcel.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/PermissionCache.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/PersistableBundle.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/ProcessInfoService.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/ProcessState.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/Status.h
    ${CMAKE_SOURCE_DIR}/external/android/include/binder/TextOutput.h
)

add_definitions(-D_GNU_SOURCE -D__ANDROID__)
add_library(binder SHARED ${SOURCES})
target_link_libraries(binder utils cutils log system-properties binderd-core)
