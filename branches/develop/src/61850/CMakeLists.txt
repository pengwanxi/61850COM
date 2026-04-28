# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.28)

message(STATUS "CMake version: " ${CMAKE_VERSION})
message(STATUS "CMake system name: " ${CMAKE_SYSTEM_NAME})

# The version number.
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(ROOTPASSWD "@adminiPDU123")
set(ESNFILE "/usr/share/esn")

file(WRITE "version" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")

set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake;${CMAKE_MODULE_PATH}")

include(env)
include(opt)
include(cc)

# 生成的最后文件的名称
set(APP iec61850)
# 项目信息
project(${APP})

if(MAKE_VERBOSE)
  message(STATUS "make verbose")
  set(CMAKE_VERBOSE_MAKEFILE on)
endif(MAKE_VERBOSE)

if(THIRD_PARTY_LIBS)
  add_subdirectory(3rd)
else()
  add_subdirectory(src)
endif(THIRD_PARTY_LIBS)

if(ENABLE_TEST)
  add_subdirectory(test)
endif(ENABLE_TEST)
# 加入一个配置头文件，用于处理 CMake 对源码的设置
configure_file("${PROJECT_SOURCE_DIR}/scripts/config.h.in"
               "${PROJECT_SOURCE_DIR}/inc/config/config.h")
