# EMU2000_Web 部署与使用说明

## 1. 目录结构
解压后目录为：/mynand/web/

- /mynand/web/html/         # 前端静态资源
- /mynand/web/config/       # 配置文件
- /mynand/web/logs/      # 日志文件
- /mynand/web/web_EMU2000.fcgi # CGI主程序
- /mynand/web/start.sh      # 启动脚本
- /mynand/web/stop.sh       # 停止脚本
- /mynand/web/gen_ssl.sh    # 生成自签名证书脚本
- /mynand/web/gen_passwd.sh # 密码工具
- /mynand/web/lighttpd.conf # Web服务配置

## 2. 证书说明
设备期望的证书文件位于：
/etc/ssl/web/server.pem
该文件必须为 PEM 格式，且同时包含服务器证书和私钥（先放证书，后放私钥）。
cat your_certificate.crt your_private.key > server.pem

## 3. 安装步骤和运行
1. 上传 package/web.tar 到设备
2. 解压到 /mynand 目录下：
   tar -xvf web.tar -C /mynand
3. 进入 /mynand/web 目录，执行安装脚本：
   cd /mynand/web && sh start.sh

## 4. 启动/停止
- 启动：sh /mynand/web/start.sh
- 停止：sh /mynand/web/stop.sh

## 5. 访问方式
- 通过 http://设备IP:18000 访问
- 通过 https://设备IP:18443 访问，浏览器会因自签名证书提示不安全，正式环境请更换证书

## 6. lighttpd.conf 说明
- 已配置好CGI、静态资源、SSL
- 如需修改端口、证书路径等，请编辑 /mynand/web/lighttpd.conf

## 7. 其他
- 日志文件：/mynand/web/logs/debug.log
- 访问日志：/mynand/web/logs/access.log

默认账号密码：admin/admin