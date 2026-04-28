# 生成的最后文件的名称
set(APP src)
# 项目信息
project(${APP})

file(GLOB_RECURSE LOCAL_COMMON_SRC "${ROOT_DIR}/src/common/*.c")
FIND_INCLUDE_DIR(LOCAL_COMMON_INC ${ROOT_DIR}/src/common/)

# cjson
FIND_INCLUDE_DIR(LOCAL_JSON_INC /usr/include/cjson/)
if("${PLAT}" STREQUAL "0")
  set(LOCAL_JSON_LIB /usr/lib/libcjson.so)
else()
  set(LOCAL_JSON_LIB ${ROOT_DIR}/lib/T113/libcjson.so)
endif()
message(STATUS "LOCAL_JSON_INC" = ${LOCAL_JSON_INC})
message(STATUS "LOCAL_JSON_LIB" = ${LOCAL_JSON_LIB})

FIND_INCLUDE_DIR(LOCAL_ZLOG_INC /usr/local/include/)
if("${PLAT}" STREQUAL "0")
  set(LOCAL_ZLOG_LIB /usr/lib/libzlog.so)
else()
  set(LOCAL_ZLOG_LIB ${ROOT_DIR}/lib/T113/libzlog.so)
endif()
message(STATUS "LOCAL_ZLOG_INC" = ${LOCAL_ZLOG_INC})
message(STATUS "LOCAL_ZLOG_LIB" = ${LOCAL_ZLOG_LIB})





# message(STATUS "common src" = ${LOCAL_COMMON_SRC})
# message(STATUS "common inc" = ${LOCAL_COMMON_SRC})

add_subdirectory(iec61850)
