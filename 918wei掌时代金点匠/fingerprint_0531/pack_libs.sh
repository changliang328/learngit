#!/bin/sh
DIROUT=$1
mkdir -p $DIROUT/arm64-v8a/hw
mkdir -p $DIROUT/armeabi-v7a/hw
echo "push libBtlAlgo"
cp arm64-v8a/libBtlAlgo.so $DIROUT/arm64-v8a/libBtlAlgo.so
cp armeabi-v7a/libBtlAlgo.so $DIROUT/armeabi-v7a/libBtlAlgo.so
echo "push libBtlFpHal"
cp arm64-v8a/libBtlFpHal.so $DIROUT/arm64-v8a/libBtlFpHal.so
cp armeabi-v7a/libBtlFpHal.so $DIROUT/armeabi-v7a/libBtlFpHal.so
echo "push libxuFPAlg"
cp arm64-v8a/libxuFPAlg.so  $DIROUT/arm64-v8a/libxuFPAlg.so
cp armeabi-v7a/libxuFPAlg.so $DIROUT/armeabi-v7a/libxuFPAlg.so
echo "push fingerprint.default.so"
cp arm64-v8a/libfingerprint.default.so $DIROUT/arm64-v8a/hw/fingerprint.default.so
cp armeabi-v7a/libfingerprint.default.so $DIROUT/armeabi-v7a/hw/fingerprint.default.so 
echo "push blestech.fingerprint.default.so" 
cp arm64-v8a/libblestech.fingerprint.default.so $DIROUT/arm64-v8a/hw/blestech.fingerprint.default.so
cp armeabi-v7a/libblestech.fingerprint.default.so $DIROUT/armeabi-v7a/hw/blestech.fingerprint.default.so
