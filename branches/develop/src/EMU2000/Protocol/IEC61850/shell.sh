make -f ArmMakefile ver=release
scp ../../../ArmRelease/lib/libIEC615.so root@192.168.0.12:/mynand/lib/
rm *.d *.o
