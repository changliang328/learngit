package com.btlfinger.service.aidl; 
import com.btlfinger.service.aidl.IFpsFingerClient;

interface IFpsFingerManager  
{  
    String getValue(String name);  
    int update(String name, String value, int attribute);
    void waitScreenOn();
    void powerDown();
    void initRegister();
    void register();
    void saveData();
    void saveDataWithName(String name);
    void isFingerUp();
    void matchUp(int nType);
    void listen(IFpsFingerClient client);
    void cancle(int isRegis);
    int mmiFpTest();
}
