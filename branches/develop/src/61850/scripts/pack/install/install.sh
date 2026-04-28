#!/bin/sh

SH_PATH=$(
	cd "$(dirname "$0")"
	pwd
)

INSTALL_DIR=/home/sysadmin/

mkdir -p /${INSTALL_DIR}/bin/
mkdir -p /${INSTALL_DIR}/lib/
mkdir -p /${INSTALL_DIR}/www
mkdir -p /${INSTALL_DIR}/log
mkdir -p /${INSTALL_DIR}/data
mkdir -p /${INSTALL_DIR}/config

cp $SH_PATH/${INSTALL_DIR}/bin/* /${INSTALL_DIR}/bin/ -rf
cp $SH_PATH/${INSTALL_DIR}/lib/* /${INSTALL_DIR}/lib/ -rf
cp $SH_PATH/${INSTALL_DIR}/www/* /${INSTALL_DIR}/www/ -rf

chmod -R 777 /${INSTALL_DIR}/bin/*
sync
