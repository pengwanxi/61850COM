#!/bin/sh
# 生成自签名SSL证书脚本
# 证书路径: /etc/ssl/web/
# 证书名: server.crt, server.key
# set -e
# CERT_DIR="/etc/ssl/web"
# mkdir -p "$CERT_DIR"
# openssl req -x509 -nodes -days 3650 -newkey rsa:2048 \
#   -keyout "$CERT_DIR/server.key" \
#   -out "$CERT_DIR/server.crt" \
#   -subj "/C=CN/ST=Test/L=Test/O=Test/OU=Test/CN=localhost"
# echo "自签名证书已生成于: $CERT_DIR"

#!/bin/bash

# 获取脚本所在目录
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

CERT_DIR="$SCRIPT_DIR/web_ssl"

echo "证书生成目录: $CERT_DIR"

# 创建目录
mkdir -p "$CERT_DIR"

# 生成证书和私钥
openssl req -x509 -newkey rsa:2048 \
    -keyout "$CERT_DIR/privkey.pem" \
    -out "$CERT_DIR/cert.pem" \
    -days 3650 \
    -nodes \
    -subj "/CN=EMU2000"

# 合并为 lighttpd 使用的 pem
cat "$CERT_DIR/cert.pem" "$CERT_DIR/privkey.pem" > "$CERT_DIR/server.pem"

# 设置权限
chmod 600 "$CERT_DIR/server.pem"

# 删除临时文件（可选）
rm -f "$CERT_DIR/cert.pem" "$CERT_DIR/privkey.pem"

echo "证书生成完成:"
echo "$CERT_DIR/server.pem"
