#!/bin/sh

PID=$(ps | grep lighttpd | grep -v grep | awk '{print $1}')

if [ -n "$PID" ]; then
    kill $PID
    echo "lighttpd已停止"
else
    echo "lighttpd未运行"
fi