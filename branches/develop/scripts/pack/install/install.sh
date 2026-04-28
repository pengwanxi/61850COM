#!/bin/sh

SH_PATH=$(
	cd "$(dirname "$0")"
	pwd
)

INSTALL_DIR=/mynand/
mkdir -p $INSTALL_DIR/firmware
STATUS_FILE=$INSTALL_DIR/firmware/upgrade_status.json
pack_name=$(basename "$SH_PATH")

# Function to update status
update_status() {
	local status=$1
	local progress=$2
	local message=$3

	echo "{\"status\":$status,\"progress\":$progress,\"message\":\"$message\",\"filename\":\"${pack_name}\",\"time\":\"$(date +'%Y-%m-%d %H:%M:%S')\"}"
	echo "{\"status\":$status,\"progress\":$progress,\"message\":\"$message\",\"filename\":\"${pack_name}\",\"time\":\"$(date +'%Y-%m-%d %H:%M:%S')\"}" >$STATUS_FILE
	sync
}

# Create initial status file
update_status 0 0 "准备安装"
update_status 1 10 "创建文件夹"
mkdir -p /${INSTALL_DIR}/bin/
mkdir -p /${INSTALL_DIR}/lib/
mkdir -p /${INSTALL_DIR}/web
mkdir -p /${INSTALL_DIR}/log
mkdir -p /${INSTALL_DIR}/data
mkdir -p /${INSTALL_DIR}/firmware
mkdir -p /${INSTALL_DIR}/downprgm
mkdir -p /${INSTALL_DIR}/FaultRecorder

update_status 1 20 "cp 文件"

echo "正在复制bin文件..."
cp $SH_PATH/${INSTALL_DIR}/bin/* /${INSTALL_DIR}/bin/ -rf
sync
echo "正在复制lib文件..."
cp $SH_PATH/${INSTALL_DIR}/lib/* /${INSTALL_DIR}/lib/ -rf
sync
echo "正在复制web文件..."
cp $SH_PATH/${INSTALL_DIR}/web/* /${INSTALL_DIR}/web/ -rf
sync
# cp $SH_PATH/${INSTALL_DIR}/web/lighttpd.conf /etc/
echo "设置init.d脚本权限..."
sync
chmod -R 755 $SH_PATH/etc/init.d/*
echo "正在复制etc文件..."
cp $SH_PATH/etc/* /etc/ -rf
sync
if [ ! -f /${INSTALL_DIR}/config/comtrade.conf ]; then
	echo "正在复制comtrade.conf"
	cp $SH_PATH/${INSTALL_DIR}/config/comtrade.conf /${INSTALL_DIR}/config/comtrade.conf -rf
fi

echo "正在复制procman.conf"
cp $SH_PATH/${INSTALL_DIR}/config/procman.conf /${INSTALL_DIR}/config/procman.conf -rf
sync

echo "正在复制procman.conf"
cp $SH_PATH/${INSTALL_DIR}/config/Stn/* /${INSTALL_DIR}/config/Stn/ -rf
sync

# 删除
update_status 1 20 "删除 文件"
if [ -f "/etc/init.d/S99epdu" ]; then
	echo "正在删除旧版S99epdu文件..."
	rm /etc/init.d/S99epdu
	sync
fi

update_status 1 40 "cfg"
if [ ! -d "/${INSTALL_DIR}/config" ]; then
	echo "创建config目录..."
	mkdir -p /${INSTALL_DIR}/config
	echo "复制config文件..."
	cp $SH_PATH/${INSTALL_DIR}/config/* /${INSTALL_DIR}/config/ -rf
	sync
fi
echo "创建iec61850配置目录..."
mkdir -p /${INSTALL_DIR}/config/iec61850
echo "复制iec61850配置文件..."
cp $SH_PATH/${INSTALL_DIR}/config/iec61850/* /${INSTALL_DIR}/config/iec61850/ -rf
sync

update_status 1 90 "权限"

echo "设置bin目录权限..."
chmod -R 777 /${INSTALL_DIR}/bin/*

# Update status to success
update_status 2 100 "升级成功"
