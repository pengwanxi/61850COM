#!/bin/bash
# 在当前文件夹下，输入./make.sh d为编入debug的程序，其它为release的程序.
   
if [ "$1"x = "d"x ];
then
	echo "make -f x86Makefile debug"
	sleep 2s
	make -f x86Makefile distclean;
	make -f x86Makefile -j4;
else
	echo "make -f x86Makefile release"
	sleep 2s
	make -f x86Makefile distclean ver=release;
	make -f x86Makefile ver=release -j4;
fi
