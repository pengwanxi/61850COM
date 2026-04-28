SCRIPT_PATH=$(
	cd "$(dirname "$0")"
	pwd
)

echo "Building project in $SCRIPT_PATH"
cd $SCRIPT_PATH/src/
make

# EMU2000
echo "Building EMU2000..."
cd $SCRIPT_PATH/src/EMU2000/
make clean
make

# 61850
echo "Building 61850..."
cd $SCRIPT_PATH/src/61850/
rm output -rf
./build.sh
./build.sh pack

# Web_EMU2000
echo "Building Web_EMU2000..."
cd $SCRIPT_PATH/src/Web/
make clean
make
make pack
make install

# NTPClient
echo "Building NTPClient..."
cd $SCRIPT_PATH/src/ntpclient/
make clean
make
make install

# KeyMonitor
echo "Building KeyMonitor..."
cd $SCRIPT_PATH/src/keymonitor/
make clean
make
make install


# 将默认文件夹cp 过去
mkdir -p $SCRIPT_PATH/output/mynand/
cp -rLf $SCRIPT_PATH/cfg/default_dir/mynand/* $SCRIPT_PATH/output/mynand/
mkdir -p $SCRIPT_PATH/output/etc/
chmod 755 $SCRIPT_PATH/cfg/default_dir/etc/init.d/*
cp -rf $SCRIPT_PATH/cfg/default_dir/etc/* $SCRIPT_PATH/output/etc/
