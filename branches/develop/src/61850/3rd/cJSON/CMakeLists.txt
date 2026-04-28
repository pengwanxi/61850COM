cmake_minimum_required(VERSION 2.8.5)

set(APP cjson)

# 项目信息
project (${APP})

# 查找当前目录下的所有源文件
aux_source_directory(./cJSON/ DIR_LIB_SRCS)

# 生成链接库
add_library (${APP} SHARED ${DIR_LIB_SRCS})

install(TARGETS ${APP} DESTINATION ${LIBRARY_DIR})
