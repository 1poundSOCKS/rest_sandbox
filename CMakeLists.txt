cmake_minimum_required(VERSION 3.25)

project(rest_sandbox_project)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(LUA_INCLUDE $ENV{LUA_INCLUDE})
set(LUA_LIB $ENV{LUA_LIB})
set(CURL_INCLUDE $ENV{CURL_INCLUDE})
set(BINARY_RESOURCES $ENV{BINARY-RESOURCES})

set(CMAKE_CXX_STANDARD 23)

set(source
  source/main.cpp
)

add_executable(rest_sandbox 
  ${source}
  ${BINARY_RESOURCES}
)

add_compile_definitions(rest_sandbox $ENV{COMPILE-DEFINITIONS})
target_compile_options(rest_sandbox PRIVATE $ENV{COMPILE-OPTIONS})
target_link_options(rest_sandbox PRIVATE $ENV{LINK-OPTIONS})

target_precompile_headers(rest_sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source/pch.h)

target_include_directories(rest_sandbox PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${LUA_INCLUDE}
  ${CURL_INCLUDE}
)
