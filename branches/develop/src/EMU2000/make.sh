#!/bin/bash
# 在当前文件夹下，输入./make.sh d为编入debug的程序，其它为release的程序.
if [ "$2"x = "d"x ];
then
	echo "make debug"
	sleep 2s
	make distclean opt=$1;
	make opt=$1;
else
	echo "make release $1"
	sleep 2s
	make distclean ver=r opt=$1;
	make ver=r opt=$1;
fi
