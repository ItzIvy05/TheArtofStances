cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ENV{COMMONLIB_SSE_FOLDER})
    message(FATAL_ERROR "Missing COMMONLIB_SSE_FOLDER environment variable")
endif()

set(AUTHOR_NAME "Nahhomie")
set(PRODUCT_NAME "Stances")

project(${PRODUCT_NAME} VERSION 0.1.0.0 LANGUAGES CXX RC)

set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "Disable automatic CMake regeneration during build" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(cmake/lib/copyOutputs.cmake)
include(cmake/lib/automaticGameFolderOutput.cmake)
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
set(ENABLE_SKYRIM_VR ON CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory("$ENV{COMMONLIB_SSE_FOLDER}" ${CMAKE_CURRENT_BINARY_DIR}/_deps/clib-build)
get_target_property(COMMONLIB_SRC_DIR CommonLibSSE SOURCE_DIR)
include(${COMMONLIB_SRC_DIR}/cmake/CommonLibSSE.cmake)

add_commonlibsse_plugin(
    ${PROJECT_NAME}
    AUTHOR "${AUTHOR_NAME}"
    SOURCES ${headers} ${sources}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h)
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(EXISTS "${PROJECT_SOURCE_DIR}/data")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/" DESTINATION ".")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE IS_HOST_PLUGIN)

# Optional auto-copy after build. Set these to true/false as needed.
set(wildlander_output false)
set(steam_owrt_output false)
set(steam_mods_output true)
set(lorerim_mods_output false)
set(lorerim3_mods_output false)

automaticGameFolderOutput(
    ${wildlander_output}
    ${steam_owrt_output}
    ${steam_mods_output}
    ${lorerim_mods_output}
    ${lorerim3_mods_output}
)
