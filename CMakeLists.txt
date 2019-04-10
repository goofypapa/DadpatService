cmake_minimum_required(VERSION 3.14)

project(DadpatService)

set(DADPATSERVICE_VERSION_MAJOR "1")
set(DADPATSERVICE_VERSION_MINOR "0")
set(DADPATSERVICE_VERSION_PATCH "0")
set(DADPATSERVICE_VERSION_EXTRA "")
set(DADPATSERVICE_VERSION ${DADPATSERVICE_VERSION_MAJOR}.${DADPATSERVICE_VERSION_MINOR})
set(DADPATSERVICE_VERSION_FULL ${DADPATSERVICE_VERSION}.${DADPATSERVICE_VERSION_PATCH}${DADPATSERVICE_VERSION_EXTRA})

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