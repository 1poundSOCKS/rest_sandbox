cmake_minimum_required(VERSION 3.24)

project(rest_sandbox_project)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
  $ENV{BOOST_ASIO_INCLUDE}
  $ENV{BOOST_BEAST_INCLUDE}
  $ENV{NLOHMANN_JSON_INCLUDE}
  $ENV{MYSQL_INCLUDE}
)

target_link_libraries(rest_sandbox PRIVATE odbc32.lib)
