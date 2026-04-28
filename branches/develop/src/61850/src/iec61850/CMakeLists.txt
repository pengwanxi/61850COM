# 生成的最后文件的名称
set(APP iec61850)

# 项目信息
project(${APP})

find_include_dir(LOCAL_INC_DIR ${ROOT_DIR}/src/iec61850/)

file(GLOB_RECURSE LOCAL_SRC "${ROOT_DIR}/src/iec61850/*.c" ${LOCAL_COMMON_SRC})

set(LOCAL_LIB ${ROOT_DIR}/lib/libiec61850/lib/libiec61850.so)
message(STATUS "LOCAL_LIB" = ${LOCAL_LIB})

include_directories(
  ${LOCAL_INC_DIR} ${ROOT_DIR}/inc/libiec61850/
  ${ROOT_DIR}/inc/config/
  ${LOCAL_COMMON_INC}
  ${LOCAL_JSON_INC}
  ${LOCAL_ZLOG_INC}
)

message(STATUS "LOCAL_INC_DIR" = ${LOCAL_INC_DIR})

aux_source_directory(. DIR_SRCS)

# 指定生成目标
add_executable(${APP} ${DIR_SRCS})

target_sources(${APP} PRIVATE ${LOCAL_SRC})

link_libraries(pthread)
find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(${APP} dl)
target_link_libraries(${APP} ${LOCAL_JSON_LIB})
target_link_libraries(${APP} ${LOCAL_ZLOG_LIB})
target_link_libraries(${APP} ${LOCAL_LIB})

# target_link_libraries(${APP} ${LIBRARY_DIR}/zlog) target_link_libraries(${APP}
# cJSON)

install(TARGETS ${APP} DESTINATION ${BINARY_DIR}/)
