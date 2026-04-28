#!/bin/bash

SPATH=$(
	cd "$(dirname "$0")"
	pwd
)

$SPATH/startup_epdu.sh
sleep 5
$SPATH/startup_web.sh
