# this is required

message(STATUS "use ${PLAT} toolchain")

set(TOOLCHAIN_PATH
    /opt/toolchain/t113/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabi)
set(TOOLCHAIN_SUFFIX ${TOOLCHAIN_PATH}/bin/arm-linux-gnueabi-)

message(STATUS "toolchain=${TOOLCHAIN_SUFFIX}")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER ${TOOLCHAIN_SUFFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_SUFFIX}g++)
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PATH})

# # where is the target environment SET(CMAKE_FIND_ROOT_PATH
# /opt/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf/)

# # search for programs in the build host directories (not necessary)
# SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) # for libraries and headers in
# the target directories SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# SET(LIB_THIRDPARTY "${CMAKE_SH_PATH}/cmake/lib/ttu/") SET(BIN_THIRDPARTY
# "${CMAKE_SH_PATH}/cmake/bin/ttu/") SET(APP_DATA_PATH
# "${CMAKE_SH_PATH}/cmake/data") SET(APP_RUN_PATH "${CMAKE_SH_PATH}/cmake/run")
# SET(INSTALL_INC "${CMAKE_SH_PATH}/cmake/inc/")

# #set extern libraries SET(LIBRARIES ${LIB_THIRDPARTY}/libcares.so
# ${LIB_THIRDPARTY}/libcrypto.so ${LIB_THIRDPARTY}/libicudata.so
# ${LIB_THIRDPARTY}/libicuuc.so ${LIB_THIRDPARTY}/liblzma.so
# ${LIB_THIRDPARTY}/libmosquitto.so ${LIB_THIRDPARTY}/libsg.so
# ${LIB_THIRDPARTY}/libssl.so ${LIB_THIRDPARTY}/libxml2.so
# ${LIB_THIRDPARTY}/libz.so )

# # include_directories( #   ${INSTALL_INC} #   )

# # set(ZLOG_CONF "\"/home/root/install/etc/zlog/zlog.conf\"")
