cmake_minimum_required(VERSION 3.8.0)

project("ezkv")

add_library(
        ezkv

        SHARED

        CodedInputData.cpp
        CodedInputData.h
        CodedOutputData.cpp
        CodedOutputData.h
        EZKV.cpp
        EZKV.h
        EZKV_Android.cpp
        EZKV_IO.cpp
        EZKV_IO.h
        EZKVLog.h
        EZKVPredef.h
        KeyValueHolder.cpp
        KeyValueHolder.h
        MemoryFile.cpp
        MemoryFile.h
        MiniPBCoder.cpp
        MiniPBCoder.h
        MMBuffer.cpp
        MMBuffer.h
        native-bridge.cpp
        PBUtility.cpp
        PBUtility.h
)

find_library(
        log-lib
        log
)

target_link_libraries(
        ezkv
        ${log-lib}
)