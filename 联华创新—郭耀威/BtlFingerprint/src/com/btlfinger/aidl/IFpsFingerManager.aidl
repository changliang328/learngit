package com.btlfinger.aidl; 
import com.btlfinger.aidl.IFpsFingerClient;

interface IFpsFingerManager  
{  
    int SetKeyCode(int keycode);
    void waitScreenOn();
    void listen(IFpsFingerClient client);
    int mmiFpTest();
}
