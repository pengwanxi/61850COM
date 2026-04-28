# ARM FastCGI Web Makefile

include ../EMU2000/config.mk
TARGET = web_EMU2000.fcgi

BUILD_DIR = build
OUTPUT_DIR = output/cgi-bin
PACKAGE_DIR = package/web

SRC = \
backend/src/web_EMU2000.c \
backend/src/config_manager.c

OBJ = \
$(BUILD_DIR)/web_EMU2000.o \
$(BUILD_DIR)/config_manager.o

INCLUDE = -Ibackend/inc
LIBDIR = lib/arm

CC_PREFIX=/opt/toolchain/t113/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-
# CC = arm-linux-gnueabi-gcc
# STRIP = arm-linux-gnueabi-strip
CC = $(CC_PREFIX)gcc
CXX = $(CC_PREFIX)g++
STRIP = $(CC_PREFIX)strip

CFLAGS = -Wall -O2 -std=c99 $(INCLUDE)

LDFLAGS = \
-L$(LIBDIR) \
-Wl,-Bstatic -lfcgi -lzlog \
-Wl,-Bdynamic -lcjson -lpthread -lcrypt

# ================================
# 编译
# ================================

all: $(OUTPUT_DIR)/$(TARGET)

$(OUTPUT_DIR)/$(TARGET): $(OBJ)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	$(STRIP) $@

$(BUILD_DIR)/web_EMU2000.o: backend/src/web_EMU2000.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/config_manager.o: backend/src/config_manager.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ================================
# 清理
# ================================

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)
	rm -rf package

# ================================
# 打包
# ================================

pack: clean all
	@echo "========== 开始打包 =========="

	mkdir -p $(PACKAGE_DIR)

	# CGI程序
	mkdir -p $(PACKAGE_DIR)/cgi-bin
	cp $(OUTPUT_DIR)/$(TARGET) $(PACKAGE_DIR)/cgi-bin/

	# HTML页面
	if [ -d frontend/html ]; then cp -r frontend/html $(PACKAGE_DIR)/; fi

	# 配置
	if [ -d config ]; then cp -r config $(PACKAGE_DIR)/; fi

	# 脚本
	if [ -d scripts ]; then cp -r scripts/start.sh $(PACKAGE_DIR)/; fi
	if [ -d scripts ]; then cp -r scripts/stop.sh $(PACKAGE_DIR)/; fi

# 	# 工具
# 	if [ -f backend/tools/gen_passwd.sh ]; then cp backend/tools/gen_passwd.sh $(PACKAGE_DIR)/; fi

	# 库
	mkdir -p $(PACKAGE_DIR)/lib
	cp lib/arm/libcjson.so* $(PACKAGE_DIR)/lib/

	# lighttpd配置
	if [ -f lighttpd.conf ]; then cp lighttpd.conf $(PACKAGE_DIR)/; fi

	#logs
	mkdir -p $(PACKAGE_DIR)/logs
	mkdir -p $(PACKAGE_DIR)/tmp

	# README
	if [ -f README.txt ]; then cp README.txt $(PACKAGE_DIR)/; fi

	cd package && tar cvf web.tar web

	@echo "========== 打包完成 =========="
	@echo "生成文件: package/web.tar"

install:
	mkdir -p $(BINARY_PATH)/../web/
	cp package/web/* $(BINARY_PATH)/../web -rf

uninstall:
	rm  $(BINARY_PATH)/../web/* -rf
