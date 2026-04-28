#!/bin/bash
SCRIPT_PATH=$(
	cd "$(dirname "$0")"
	pwd
)

cd $SCRIPT_PATH/scripts/build/
$SCRIPT_PATH/scripts/build/build_opt.sh $@
