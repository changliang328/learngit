#!/bin/sh
DIROUT=$1
SOURCE_DIR=$(cd `dirname $0` ; pwd)
TARGET_DIR=${SOURCE_DIR}/out/release
mkdir -p $DIROUT/arm64-v8a/hw
mkdir -p $DIROUT/armeabi-v7a/hw
echo "push libBtlAlgo"
cp $TARGET_DIR/arm64-v8a/libBtlAlgo.so $DIROUT/arm64-v8a/libBtlAlgo.so
cp $TARGET_DIR/armeabi-v7a/libBtlAlgo.so $DIROUT/armeabi-v7a/libBtlAlgo.so
echo "push libBtlFpHal"
cp $TARGET_DIR/arm64-v8a/libBtlFpHal.so $DIROUT/arm64-v8a/libBtlFpHal.so
cp $TARGET_DIR/armeabi-v7a/libBtlFpHal.so $DIROUT/armeabi-v7a/libBtlFpHal.so
echo "push libxuFPAlg"
cp $TARGET_DIR/arm64-v8a/libxuFPAlg.so  $DIROUT/arm64-v8a/libxuFPAlg.so
cp $TARGET_DIR/armeabi-v7a/libxuFPAlg.so $DIROUT/armeabi-v7a/libxuFPAlg.so
echo "push fingerprint.default.so"
cp $TARGET_DIR/arm64-v8a/fingerprint.default.so $DIROUT/arm64-v8a/hw/fingerprint.default.so
cp $TARGET_DIR/armeabi-v7a/fingerprint.default.so $DIROUT/armeabi-v7a/hw/fingerprint.default.so 
echo "push blestech.fingerprint.default.so" 
cp $TARGET_DIR/arm64-v8a/blestech.fingerprint.default.so $DIROUT/arm64-v8a/hw/blestech.fingerprint.default.so
cp $TARGET_DIR/armeabi-v7a/blestech.fingerprint.default.so $DIROUT/armeabi-v7a/hw/blestech.fingerprint.default.so
