cmake_minimum_required(VERSION 3.10)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message("Bootstrapping vcpkg in $ENV{VCPKG_ROOT} : ${NODE_ARCH}")
  if (${NODE_ARCH} STREQUAL "x64")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  else()
    set(VCPKG_TARGET_TRIPLET "x86-windows-static")
  endif()
  set(CMAKE_PREFIX_PATH $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(mzip)

if (WIN32)
  add_compile_definitions(_WIN32_WINNT=0x600 _WIN32_WINDOWS=0x600 _UNICODE UNICODE)
endif()

include_directories(${CMAKE_JS_INC})

file(GLOB SOURCE_FILES "*.cc")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})

# define NPI_VERSION
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_VERSION=3)

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

if (WIN32)
find_package(ZLIB)
else()
set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
message(STATUS "CMAKE_FIND_LIBRARY_SUFFIXES: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
find_package(ZLIB)
endif()

add_subdirectory(third_party/minizip)
add_subdirectory(third_party/fmt)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)
target_link_libraries(${PROJECT_NAME} PRIVATE minizip)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# Turn on exporting compile commands json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
ENDIF()
