#!/bin/sh
# 一键部署并启动EMU2000 Web服务
# 解压后直接运行本脚本即可
set -e
INSTALL_DIR="/mynand/web"

# 1. 赋权，确保lighttpd可访问
chmod -R a+rx "$INSTALL_DIR"

# 2. 生成自签名证书（如不存在）
#sh "$INSTALL_DIR/gen_ssl.sh"

# 3. 启动lighttpd（前台模式）
sh "$INSTALL_DIR/start.sh"
