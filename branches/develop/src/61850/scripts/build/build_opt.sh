#!/bin/bash

source helpinfo.sh
source build_env.sh
source build_cmds.sh

# oprate ######################################################################
while [ $# -gt 0 ]; do
	OPRATE=1

	case "$1" in
	make*)
		exec_make_cmd
		;;
	install*)
		exec_make_install_cmd
		;;
	pack*)
		source $SH_PATH/../pack/pack.sh
		pack_all

		# 增加复制
		cp $INSTALL_PREFIX/install/bin/* $ROOT_PATH/../../output/mynand/bin/ -rf
		cp $INSTALL_PREFIX/install/lib/*.so $ROOT_PATH/../../output/mynand/lib/ -rf
		cp $INSTALL_PREFIX/install/lib/libiec61850/lib/* $ROOT_PATH/../../output/mynand/lib/ -rsf
		cp $INSTALL_PREFIX/install/lib/libiec61850/lib/* $ROOT_PATH/../../output/mynand/lib/ -rsf
		mkdir -p $ROOT_PATH/../../output/mynand/config/iec61850
		cp $INSTALL_PREFIX/install/config/iec61850/* $ROOT_PATH/../../output/mynand/config/iec61850 -rf
		;;
	*)
		OPRATE=0
		;;
	esac

	if [ $OPRATE -eq 1 ]; then
		exit 0
	else
		break
	fi

done

# option ######################################################################
while getopts "a:dhi:l:p:PtTV" arg; do
	case $arg in
	h)
		helpinfo
		exit
		;;
	i)
		echo "create install prefix"
		mkdir -p $OPTARG
		if [ ! -d $OPTARG ]; then
			echo "create $OPTARG failed"
		else
			echo "create $OPTARG success"
			INSTALL_PREFIX=$OPTARG
		fi
		;;
	l)
		case $OPTARG in
		b)
			ELOG_BUFFER_ENABLE=ON
			;;
		esac
		;;
	p)
		PLAT=$OPTARG
		;;
	P)
		MAKE_PACKAGE=ON
		;;
	d)
		BUILD_TYPE=Debug
		;;
	t)
		ENABLE_TEST=ON
		;;
	T)
		THIRD_PARTY_LIBS=ON
		;;
	V)
		MAKE_VERBOSE=ON
		;;

	esac
done

cd $ROOT_PATH/lib/
rm libiec61850
ln -s ./$PLAT/libiec61850 libiec61850

if [ $PLAT == "desk" ]; then
	PLAT=0
else
	PLAT=1
fi

exec_cmake_cmd
exec_make_cmd
exec_make_install_cmd


# build_option
