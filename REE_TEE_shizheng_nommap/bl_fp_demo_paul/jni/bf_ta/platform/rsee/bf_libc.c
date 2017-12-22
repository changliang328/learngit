#include<stdint.h>

char* strncpy(char* dest, const char* src, int len);

char* strncpy(char* dest, const char* src, int len)  

{  
	//assert(dest!=NULL && src!=NULL);  

	char* temp=dest;  

	int i=0;  

	while(i++ < len  && (*temp++ = *src++)!='\0')  

	{}  

	if(*(temp)!='\0')  

		*temp='\0';  

	return dest;  

}  
