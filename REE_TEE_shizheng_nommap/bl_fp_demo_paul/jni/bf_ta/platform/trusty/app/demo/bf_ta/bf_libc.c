#include<stdint.h>

float sqrt(float x);
float  atan2(float y, float x, int infNum);
 

float atan2(float y, float x, int infNum)
{
	int i;
	float PI=3.14159265;
	float z = y / x, sum = 0.0f,temp;
	float del = z / infNum;

	for (i = 0; i < infNum;i++)
	{
		z = i*del;
		temp = 1 / (z*z + 1) * del;
		sum += temp;
	}

	if (x>0)
	{
		return sum;
	}
	else if (y >= 0 && x < 0)
	{
		return sum + PI;
	}
	else if (y < 0 && x < 0)
	{
		return sum - PI;
	}
	else if (y > 0 && x == 0)
	{
		return PI / 2;
	}
	else if (y < 0 && x == 0)
	{
		return -1 * PI / 2;
	}
	else
	{
		return 0;
	}
}


float sqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f375a86 - (i>>1);
    x = *(float*)&i;
    x = x*(1.5f-xhalf*x*x);
    x = x*(1.5f-xhalf*x*x);
    x = x*(1.5f-xhalf*x*x);
    return 1/x;
}





