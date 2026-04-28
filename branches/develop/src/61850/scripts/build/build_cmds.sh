#!/bin/bash

function exec_cmake_cmd {
	show_env
	mkdir -p $BUILD_PATH
	cd $BUILD_PATH

	echo -e "\n begin cmake $ROOT_PATH"

	cmake $ROOT_PATH \
	      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $ROOT_PATH \
	      -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
	      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
	      -DPLAT=$PLAT \
	      -DENABLE_TEST=$ENABLE_TEST \
	      -DELOG_BUFFER_ENABLE=$ELOG_BUFFER_ENABLE \
	      -DELOG_FILE_ENABLE=$ELOG_FILE_ENABLE \
	      -DTHIRD_PARTY_LIBS=$THIRD_PARTY_LIBS \
	      -DMAKE_VERBOSE=$MAKE_VERBOSE

	if [ $? -ne 0 ]; then
		echo -e "\n\n\n cmake error \n\n\n"
		exit
	fi

	cp $INSTALL_PREFIX/build/compile_commands.json $ROOT_PATH/compile_commands.json
}

function exec_make_cmd {
	cd $BUILD_PATH
	echo -e "\n begin make"
	make -j4
	if [ $? -ne 0 ]; then
		echo -e "\n\n\n make error \n\n\n"
		exit
	fi
}

function exec_make_install_cmd {
	cd $BUILD_PATH
	echo -e "\n begin make install"
	make install

	if [ $? -ne 0 ]; then
		echo -e "\n\n\n make install error \n\n\n"
		exit
	fi

}
