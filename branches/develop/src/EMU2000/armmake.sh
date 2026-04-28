#!/bin/bash
# 在当前文件夹下，输入./make.sh d为编入debug的程序，其它为release的程序.

if [ "$1"x = "d"x ];
then
	echo "make -f ArmMakefile debug"
	sleep 2s
	make -f ArmMakefile distclean;
	make -f ArmMakefile -j4;
else
	echo "make -f ArmMakefile release"
	sleep 2s
	make -f ArmMakefile distclean ver=release;
	make -f ArmMakefile ver=release -j4;
fi
