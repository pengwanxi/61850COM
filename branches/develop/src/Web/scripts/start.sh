#!/bin/sh
# 启动脚本：使用系统 lighttpd 服务运行 web 程序
INSTALL_DIR="/mynand/web"
LIGHTTPD_CONF="$INSTALL_DIR/lighttpd.conf"
SYS_LIGHTTPD_CONF="/etc/lighttpd/lighttpd.conf"

# 检查自定义配置文件是否存在
if [ ! -f "$LIGHTTPD_CONF" ]; then
    echo "错误: 未找到 $LIGHTTPD_CONF"
    exit 1
fi

# 备份系统原有配置文件（如果存在）
if [ -f "$SYS_LIGHTTPD_CONF" ]; then
    cp "$SYS_LIGHTTPD_CONF" "$SYS_LIGHTTPD_CONF.bak"
    echo "已备份原配置文件到 $SYS_LIGHTTPD_CONF.bak"
fi

# 复制自定义配置文件到系统路径
cp "$LIGHTTPD_CONF" "$SYS_LIGHTTPD_CONF"
echo "已复制配置文件到 $SYS_LIGHTTPD_CONF"

# 设置动态库搜索路径，确保lib/arm下so库可用
export LD_LIBRARY_PATH="/mynand/web/lib:$LD_LIBRARY_PATH"

# 设置.fcgi权限
chmod +x $INSTALL_DIR/cgi-bin/web_EMU2000.fcgi

# 重启lighttpd
/etc/init.d/S50lighttpd restart