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
	echo "cp $ROOT_PATH/lib/T113/*.so $PACK_PATH/lib/ -rf "
	cp $ROOT_PATH/lib/T113/*.so $PACK_PATH/lib/ -rf
	mkdir -p $PACK_PATH/lib/libiec61850/lib
	echo "cp $ROOT_PATH/lib/libiec61850/lib/*.so* $PACK_PATH/lib/libiec61850/lib/ -rf"
	cp $ROOT_PATH/lib/libiec61850/lib/*.so* $PACK_PATH/lib/libiec61850/lib/ -rf
}

function pack_bin {
	mkdir -p $PACK_PATH/bin
	echo "cp $ROOT_PATH/bin/ $PACK_PATH/ -rf"
	cp $ROOT_PATH/bin/ $PACK_PATH/ -rf
}

function pack_cfg {
	mkdir -p $PACK_PATH/config/iec61850
	echo "cp $ROOT_PATH/cfg/iec61850/* $PACK_PATH/config/iec61850/ -rf"
	cp $ROOT_PATH/cfg/iec61850/* $PACK_PATH/config/iec61850/ -rf
}

function pack_data {
	mkdir -p $PACK_PATH/data
}

function pack_all {
	echo -e "\n begin pack_all"
	rm $PACK_PATH/* -rf
	pack_bin
	pack_lib
	pack_cfg
	pack_data
	mkdir -p $PACK_PATH/log

	echo -e "\n end pack_all"

	cd $OUTPUT_PATH

	mkdir -p install
	mkdir -p install/root/
	cp packages/* install/ -rf

}
