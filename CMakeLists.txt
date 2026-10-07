cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ENV{COMMONLIB_SSE_FOLDER})
    message(FATAL_ERROR "Missing COMMONLIB_SSE_FOLDER environment variable")
endif()

set(AUTHOR_NAME "Ivy")
set(PRODUCT_NAME "Stances")

project(${PRODUCT_NAME} VERSION 1.0.0.0 LANGUAGES CXX RC)

set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "Disable automatic CMake regeneration during build" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(cmake/lib/copyOutputs.cmake)
include(cmake/lib/autoCollectSources.cmake)

add_src_folder(src)
add_include_folder(include)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/version.rc)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${headers} ${sources}
)

set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(REX_OPTION_INI ON CACHE BOOL "" FORCE)

add_subdirectory("$ENV{COMMONLIB_SSE_FOLDER}" ${CMAKE_CURRENT_BINARY_DIR}/_deps/clib-build)
get_target_property(COMMONLIB_SRC_DIR CommonLibSSE SOURCE_DIR)
include(${COMMONLIB_SRC_DIR}/cmake/CommonLibSSE.cmake)

find_path(MODCONFIGUI_INCLUDE_DIRS "ModConfigUI/ModConfigUI.h")
set(MODCONFIGUI_SOURCE_DIR "${MODCONFIGUI_INCLUDE_DIRS}/../share/modconfigui")
file(GLOB MODCONFIGUI_SOURCES "${MODCONFIGUI_SOURCE_DIR}/src/*.cpp")
add_library(ModConfigUI STATIC ${MODCONFIGUI_SOURCES})
target_include_directories(ModConfigUI PUBLIC "${MODCONFIGUI_INCLUDE_DIRS}")
target_link_libraries(ModConfigUI PUBLIC CommonLibSSE::CommonLibSSE)
target_compile_features(ModConfigUI PRIVATE cxx_std_23)
target_precompile_headers(ModConfigUI PRIVATE "${MODCONFIGUI_SOURCE_DIR}/PCH.h")

add_commonlibsse_plugin(
    ${PROJECT_NAME}
    AUTHOR "${AUTHOR_NAME}"
    SOURCES ${headers} ${sources}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h)
target_link_libraries(${PROJECT_NAME} PRIVATE ModConfigUI)
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(EXISTS "${PROJECT_SOURCE_DIR}/data")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ".")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    copyOutputs("$ENV{SKYRIM_MODS_FOLDER}/${PRODUCT_NAME}")
endif()
