#!/bin/bash

SH_PATH=$(
	cd "$(dirname "$0")"
	pwd
)
ROOT_PATH=$(
	cd $SH_PATH/../../
	pwd
)
BUILD_PATH=$ROOT_PATH/output/build

INSTALL_PREFIX=$ROOT_PATH/output/
BUILD_TYPE=Release
PLAT=T113
TTU=ON
ENABLE_TEST=OFF
ELOG_BUFFER_ENABLE=OFF
ELOG_FILE_ENABLE=ON
THIRD_PARTY_LIBS=OFF
MAKE_VERBOSE=OFF
MAKE_PACKAGE=OFF

function show_env {
	echo "ROOT_PATH:             $ROOT_PATH"
	echo "SH_PATH:               $SH_PATH"
	echo "BUILD_PATH:            $BUILD_PATH"
	echo "INSTALL_PREFIX:        $INSTALL_PREFIX"
	echo "PLATFORM :             $PLAT"
	echo "BUILD_TYPE:            $BUILD_TYPE"
	echo "ENABLE_TEST:           $ENABLE_TEST"
	echo "ELOG_BUFFER_ENABLE:    $ELOG_BUFFER_ENABLE"
	echo "ELOG_FILE_ENABLE:      $ELOG_FILE_ENABLE"
	echo "THIRD_PARTY_LIBS:      $THIRD_PARTY_LIBS"
	echo "MAKE_VERBOSE:          $MAKE_VERBOSE"
}
