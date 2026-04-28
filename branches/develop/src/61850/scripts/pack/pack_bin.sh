#!/bin/sh

SH_PATH=$(
	cd "$(dirname "$0")"
	pwd
)

ROOT_PATH=$(
	cd $SH_PATH/../../
	pwd
)

OUTPATH=$ROOT_PATH/output
OUTBINPATH=$ROOT_PATH/output/bin
INSTALLIPDU=$ROOT_PATH/output/install/ipdu

mkdir -p $OUTBINPATH

if [ ! -d $INSTALLIPDU ]; then
	exit -1
fi

cp $INSTALLIPDU $OUTBINPATH -rf
cp $SH_PATH/install/install.sh $OUTBINPATH/ipdu
cp $SH_PATH/all_in_one $OUTBINPATH
cp $SH_PATH/ipdu.sh $OUTBINPATH
cp $SH_PATH/pack_bin_ipdu.sh $OUTBINPATH

cd $OUTBINPATH

VER=$(cat ${ROOT_PATH}/version)
echo $VER
NAME=${VER}_${BUILD_ID}
echo $NAME

chmod +x *
./pack_bin_ipdu.sh $NAME ipdu
