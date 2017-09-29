	FOR Android4.4
1.frameworks\base\packages\SystemUI\res\layout\super_status_bar.xml   中移植：
	<include
            layout="@layout/status_bar_fp_identify"
             />
2.\frameworks\base\packages\SystemUI\res\values\strings.xml  中移植：
	<string name="fp_indicator_label">Please enter fingerpint!</string>
    <string name="fp_indicator_label_error">Incorrect fingerprint,please try again!</string>
    <string name="fp_match_label_error">Incorrect fingerprint,please try again</string>
3.frameworks\base\packages\SystemUI\res\values-zh-rCN\strings.xml  中移植;
	<string name="fp_indicator_label">请输入指纹</string>
    <string name="fp_indicator_label_error">请输入正确的指纹</string>
    <string name="fp_match_label_error">指纹输入错误,请再试</string>
4.Android.mk 中的LOCAL_SRC_FILES只有这一种形式，没有32位和64位的区分
5.apk中AndroidManifest.xml  
 <category android:name="android.intent.category.DEFAULT" />  DEFAULT修改为LUNCH属性，可以直接在桌面生成图标
6.android4.4没有使用.te的安全机制，但是需要移植.rc chmod 777 dev/bl229x
	device中的PRODUCT_PACKAGES += 库名  需要移植到其他的.mk
7.\frameworks\base\packages\SystemUI\src\com\android\systemui\statusbar\phone\PhoneStatusBar.java 中
			//blestech start
    879行    //mFingerprintView = mStatusBarWindow.findViewById(R.id.fp_identify);
			//mFpStatusLabel = (TextView) mFingerprintView.findViewById(R.id.fp_status);
			//blestech end
      此处调用错误