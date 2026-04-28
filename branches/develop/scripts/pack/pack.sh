#!/bin/bash

SH_PATH=$(
	cd "$(dirname "$0")"
	pwd
)

ROOT_PATH=$(
	cd $SH_PATH/../../
	pwd
)

PACK_PATH=$ROOT_PATH/output/packages
OUTPUT_PATH=$ROOT_PATH/output

mkdir -p $PACK_PATH

function pack_lib {
	mkdir -p $PACK_PATH/lib
	cp $ROOT_PATH/lib/libnetsnmp*.so $PACK_PATH/lib/
	# cp $ROOT_PATH/lib/ $PACK_PATH/ -rf
}

function pack_bin {
	mkdir -p $PACK_PATH/bin
	cp $ROOT_PATH/bin/ $PACK_PATH/ -rf
	cp $ROOT_PATH/arch/sysconfig $PACK_PATH/bin/ -rf
	cp $SH_PATH/../pack/startup.sh $PACK_PATH/bin/
}

function pack_www {
	mkdir -p $PACK_PATH/www
	cp $ROOT_PATH/www/ $PACK_PATH/ -rf
}

function pack_cfg {
	mkdir -p $PACK_PATH/config
	# cp $ROOT_PATH/arch/config $PACK_PATH/ -rf
}

function pack_data {
	mkdir -p $PACK_PATH/data
}

function pack_all {
	echo -e "\n begin pack_all"
	rm $PACK_PATH/* -rf
	pack_bin
	pack_lib
	pack_www
	pack_cfg
	pack_data
	mkdir -p $PACK_PATH/log

	echo -e "\n end pack_all"

	cd $OUTPUT_PATH

	mkdir -p install
	mkdir -p install/ipdu
	mkdir -p install/ipdu/home/sysadmin
	cp packages/* install/ipdu/home/sysadmin/ -rf

	tarname=ipdu.tar
	echo -e "\n tar -cvf $tarname install/ipdu "
	cd install
	tar cvf  $tarname ipdu
	mv $tarname ../
	cd -
}
