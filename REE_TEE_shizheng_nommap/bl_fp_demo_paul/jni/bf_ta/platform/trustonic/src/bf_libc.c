#include<stdint.h>
#include <taStd.h>
#include "bf_tee_plat_func.h"

void   *malloc(size_t size);
void   free(void* ptr);
void   *memcpy(void *__restrict s1, const void *__restrict s2, size_t n);
void   *memmove(void *s1, const void *s2, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
void   *memset(void *s, int c, size_t n);
int    strcmp(const char *s1, const char *s2);
size_t strlen(const char *s);
char   *strcpy(char *strDest, const char *strSrc); 
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



void *malloc(size_t size)
{
	return bf_tee_malloc(size);
}

void free(void* ptr) 
{
	bf_tee_free(ptr);
}


void   *memcpy(void *__restrict s1, const void *__restrict s2, size_t n)
{
	bf_tee_memcpy(s1,s2,n);
	return s1;
}



void   *memmove(void *s1, const void *s2, size_t n)
{
	bf_tee_memcpy(s1,s2,n);
	return s1;
}



int    memcmp(const void *s1, const void *s2, size_t n)
{
	return bf_tee_memcmp((void *)s1, (void *)s2, n);
}




void   *memset(void *s, int c, size_t n)
{
	bf_tee_memset(s,c,n);
	return s;
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





int    strcmp(const char *s1, const char *s2)
{
	assert((*s1!='\0' && *s2!='\0'));
	while (*s1 && *s2 && *s1==*s2) {
		s1++;
		s2++;
	}
	return *s1-*s2;

}



size_t strlen(const char *s)
{
	assert(s != NULL);
	int len = 0;
	while (*s++ != '\0') {
		++len;
	}
	return len;
}



char *strcpy(char *strDest, const char *strSrc) {
	char *address=strDest;
	assert((strDest!=NULL) &&  (strSrc!=NULL));
	while(*strSrc!='\0') {
		*strDest++=*strSrc++;
	}
	*strDest='\0';
	return address;
}



