#!/bin/sh
# 安装脚本：解压、初始化目录、复制配置、生成证书
set -e
INSTALL_DIR="/mynand/web"
PKG_DIR="$(dirname "$0")"

# 1. 创建目标目录
mkdir -p "$INSTALL_DIR"

# 2. 拷贝所有文件到目标目录
cp -r $PKG_DIR/html $INSTALL_DIR/
cp -r $PKG_DIR/config $INSTALL_DIR/
cp -r $PKG_DIR/docs $INSTALL_DIR/
cp $PKG_DIR/web_EMU2000.fcgi $INSTALL_DIR/
cp $PKG_DIR/gen_passwd.sh $INSTALL_DIR/
cp $PKG_DIR/start.sh $INSTALL_DIR/
cp $PKG_DIR/stop.sh $INSTALL_DIR/
cp $PKG_DIR/gen_ssl.sh $INSTALL_DIR/
cp $PKG_DIR/lighttpd.conf $INSTALL_DIR/
cp $PKG_DIR/README.txt $INSTALL_DIR/


# 3. 赋予web目录及其下所有文件可读可执行权限，确保lighttpd可访问
chmod -R a+rx $INSTALL_DIR

# 4. 生成自签名证书（如不存在）
sh $INSTALL_DIR/gen_ssl.sh

echo "安装完成，web目录已准备好。"
