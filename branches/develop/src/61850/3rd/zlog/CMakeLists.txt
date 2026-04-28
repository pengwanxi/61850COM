# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.1)

set(APP zlog)
# 项目信息
project (${APP})

# 查找当前目录下的所有源文件
aux_source_directory(zlog/src DIR_LIB_SRCS)

#增加头文件
include_directories(
  ./zlog/src
  )
# set(Need_THREAD 1)
link_libraries(pthread)
# 生成链接库
add_library (${APP} SHARED ${DIR_LIB_SRCS})

install(TARGETS ${APP} DESTINATION ${LIBRARY_DIR})
