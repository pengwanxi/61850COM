# 生成的最后文件的名称
set(APP 3rd)
# 项目信息
project(${APP})

set(IEC_PATH "${PROJECT_SOURCE_DIR}/libiec61850/build")
set(IEC_CLEAN "rm -rf ${IEC_PATH}/*" )

if(${PLAT} STREQUAL "0")
  set(IEC_MAKE "make clean && make && cp ${IEC_PATH}../.install/lib/*.so ${IEC_PATH}../.install/lib/*.so.* ${PROJECT_SOURCE_DIR}/../lib/desk/")
  message(STATUS "use desk toolchain")
  message(STATUS "export TOOLCHAIN=/usr/bin/;  ${IEC_CLEAN}; cd ${IEC_PATH};cmake .. ${IEC_INSTALL} && ${IEC_MAKE}")
  execute_process(
    COMMAND
      sh -c
      "export TOOLCHAIN=/usr/bin/; ${IEC_CLEAN}; cd ${IEC_PATH};cmake ..   ${IEC_ISNTALL} && ${IEC_MAKE}"
  )

  message(STATUS "toolchain = ENV{TOOLCHAIN}")

else(${PLAT} STREQUAL "1")
  add_subdirectory(cJSON)
  add_subdirectory(zlog)

  set(IEC_MAKE "make clean && make && cp ${IEC_PATH}/../.install/lib/*.so ${IEC_PATH}/../.install/lib/*.so.* ${PROJECT_SOURCE_DIR}/../lib/T113/")

  message(STATUS "IEC_MAKE = ${IEC_MAKE}")

  message(STATUS "use T113 toolchain")
  message(STATUS "export TOOLCHAIN=/opt/toolchain/t113/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-; ${IEC_CLEAN}; cd ${IEC_PATH};cmake ..   ${IEC_INSTALL} && ${IEC_MAKE}")
  execute_process(
    COMMAND
      sh -c
      "TOOLCHAIN=/opt/toolchain/t113/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi- ${IEC_CLEAN}; cd ${IEC_PATH};cmake ..   ${IEC_INSTALL} && ${IEC_MAKE}"
  )
endif(${PLAT} STREQUAL "0")
