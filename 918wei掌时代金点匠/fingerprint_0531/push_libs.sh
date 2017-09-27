#!/bin/sh
adb remount
echo "push libBtlAlgo"
adb push arm64-v8a/libBtlAlgo.so /system/lib64/libBtlAlgo.so
adb push armeabi-v7a/libBtlAlgo.so /system/lib/libBtlAlgo.so
echo "push libBtlFpHal"
adb push arm64-v8a/libBtlFpHal.so /system/lib64/libBtlFpHal.so
adb push armeabi-v7a/libBtlFpHal.so /system/lib/libBtlFpHal.so
echo "push libxuFPAlg"
adb push arm64-v8a/libxuFPAlg.so  /system/lib64/libxuFPAlg.so
adb push armeabi-v7a/libxuFPAlg.so /system/lib/libxuFPAlg.so
echo "push fingerprint.default.so"
adb push arm64-v8a/libfingerprint.default.so /system/lib64/hw/fingerprint.default.so
adb push armeabi-v7a/libfingerprint.default.so /system/lib/hw/fingerprint.default.so 
echo "push blestech.fingerprint.default.so" 
adb push arm64-v8a/libblestech.fingerprint.default.so /system/lib64/hw/blestech.fingerprint.default.so
adb push armeabi-v7a/libblestech.fingerprint.default.so /system/lib/hw/blestech.fingerprint.default.so
