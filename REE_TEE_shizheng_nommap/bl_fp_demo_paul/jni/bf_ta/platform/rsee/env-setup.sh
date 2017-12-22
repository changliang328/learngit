export SDK_32BIT_MODE=y
if [ $SDK_32BIT_MODE == y ]
then
	echo "compile 32 bit"
export CROSS_COMPILE=arm-none-eabi-
else
	echo "compile 64 bit"
export CROSS_COMPILE=aarch64-linux-android-
fi
export TA_DEV_KIT_DIR=/home/musk/TEE/rongka/rsdk/rsee_sdk-ta_arm64
export COMPILE_PATH_32BIT=/home/musk/TEE/rongka/rsdk/gcc/linux-x86/arm/gcc-arm-none-eabi-4_8-2014q3/bin
export COMPILE_PATH_64BIT=/home/musk/TEE/rongka/rsdk/gcc/linux-x86/aarch64/aarch64-linux-android-4.8/bin
#export PATH=$PATH:$COMPILE_PATH_32BIT:$COMPILE_PATH_64BIT
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:$COMPILE_PATH_32BIT:$COMPILE_PATH_64BIT
