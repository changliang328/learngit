#ifndef _LIB_TVM_TIME_H
#define _LIB_TVM_TIME_H
#ifdef __cplusplus
extern "C" {
#endif

//#include <contrib/lib_virtualization/teei_tvm.h>
#ifndef UINT
typedef unsigned int	UINT;
#endif

typedef struct
{
	UINT seconds;
	UINT mills;
} TIME_Time;

    ////打开
	//int Lib_timeOpen();
	////关闭
	//int Lib_timeClose();
	//获取当前的系统时钟
	UINT GetSystemTime(TIME_Time* time);
	//获取REE的时间
    UINT GetREETime(TIME_Time* time);
	//等待一段时间
	UINT ut_msleep(UINT timeout);
    UINT SetSystemSwitchFlag();
    UINT GetSystemTimeQuick(TIME_Time* time);
    int GetExactTime(UINT *high, UINT *low);
#ifdef __cplusplus
}
#endif

#endif

