adb wait-for-device
adb root
adb remount

while [ 1 ];do
#adb shell "cat /proc/kmsg "
adb shell "cat /proc/tkcore/tkcore_log "
done
