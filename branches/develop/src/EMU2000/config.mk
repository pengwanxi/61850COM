# 平台配置相关，每个文件都要依赖

PLATFORM=T113
# PLATFORM=desk
#VER=DEBUG

makefile_path := $(lastword $(MAKEFILE_LIST))  # 获取当前 Makefile 文件路径
SRC_ROOT_DIR:= $(dir $(realpath $(makefile_path)))

ROOT_DIR:= $(SRC_ROOT_DIR)/../../
ROOT_OUTPUT_DIR:= $(SRC_ROOT_DIR)/../../output


ifeq ($(PLATFORM), T113)
CC_PREFIX=/opt/toolchain/t113/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-

CC = $(CC_PREFIX)gcc
CXX = $(CC_PREFIX)g++

LIB_61850 = $(ROOT_DIR)/lib/libiec61850.so

endif

ifeq ($(PLATFORM), desk)

CC = $(CC_PREFIX)gcc
CXX = $(CC_PREFIX)g++
LIB_61850 = $(ROOT_DIR)/lib/desk/libiec61850.so

endif

DEBUG_BIN_PATH := $(ROOT_OUTPUT_DIR)/$(PLATFORM)/debug/bin
DEBUG_LIB_PATH := $(ROOT_OUTPUT_DIR)/$(PLATFORM)debug/lib
DEBUG_OBJ_PATH := $(ROOT_OUTPUT_DIR)/$(PLATFORM)/obj

RELEASE_BIN_PATH := $(ROOT_OUTPUT_DIR)/$(PLATFORM)/release/bin
RELEASE_LIB_PATH := $(ROOT_OUTPUT_DIR)/$(PLATFORM)/release/lib
RELEASE_OBJ_PATH := $(ROOT_OUTPUT_DIR)/$(PLATFORM)/obj


BINARY_PATH := $(RELEASE_BIN_PATH)
LIBRARY_PATH := $(RELEASE_LIB_PATH)
OBJECT_PATH := $(RELEASE_OBJ_PATH)
CFLAGS  =  -O2
CXXFLAGS=  -O2
ifeq ($(VER), DEBUG)
CFLAGS  = -g
CXXFLAGS= -g
BINARY_PATH := $(DEBUG_BIN_PATH)
LIBRARY_PATH := $(DEBUG_LIB_PATH)
OBJECT_PATH := $(DEBUG_OBJ_PATH)
endif

$(shell mkdir -p $(OBJECT_PATH))
$(shell mkdir -p $(BINARY_PATH))
$(shell mkdir -p $(LIBRARY_PATH))

# print:
# 	@echo "Makefile absolute directory: $(SRC_ROOT_DIR)"
# 	@echo "Debug Binary Path: $(DEBUG_BIN_PATH)"
# 	@echo "Debug Library Path: $(DEBUG_LIB_PATH)"
# 	@echo "Release Binary Path: $(RELEASE_BIN_PATH)"
# 	@echo "Release Library Path: $(RELEASE_LIB_PATH)"
