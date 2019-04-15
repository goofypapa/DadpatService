cmake_minimum_required(VERSION 3.14)

set(DADPATSERVICE_VERSION_MAJOR "1")
set(DADPATSERVICE_VERSION_MINOR "0")
set(DADPATSERVICE_VERSION_PATCH "0")
set(DADPATSERVICE_VERSION_EXTRA "")
set(DADPATSERVICE_VERSION ${DADPATSERVICE_VERSION_MAJOR}.${DADPATSERVICE_VERSION_MINOR})
set(DADPATSERVICE_VERSION_FULL ${DADPATSERVICE_VERSION}.${DADPATSERVICE_VERSION_PATCH}${DADPATSERVICE_VERSION_EXTRA})

project(DadpatService VERSION ${DADPATSERVICE_VERSION})

option(X86_64 "编译x86_64架构程序" ON)

set(CMAKE_C_COMPILER arm-linux-gcc)
set(CMAKE_CXX_COMPILER arm-linux-g++)

if(X86_64)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
else()
    # this is required
    set(CMAKE_SYSTEM_NAME Linux)

    # specify the cross compiler
    set(CMAKE_C_COMPILER   arm-linux-gnueabi-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabi-g++)

    # add_definitions(-std=c++11)

    # search for programs in the build host directories (not necessary)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    # for libraries and headers in the target directories
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

endif()


set(CMAKE_BUILD_TYPE Debug CACHE STRING "set build type to debug")

aux_source_directory(src SOURCE)
aux_source_directory(tools/src TOOLS_SOURCE)
aux_source_directory(audio/src AUDIO_SOURCE)

add_library( Tools ${TOOLS_SOURCE} )
add_library( Audio ${AUDIO_SOURCE} )

include_directories(audio tools)

link_libraries( Audio Tools asound pthread )

add_executable(${PROJECT_NAME}_${DADPATSERVICE_VERSION_FULL} ${SOURCE})

FILE(COPY resource/audios DESTINATION ./)