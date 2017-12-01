/*************************************************************************
	> File Name: spiTest.c
	> Author: cl
	> Mail: changliang328@163.com 
	> Created Time: 2017年11月16日 星期四 14时14分34秒
 ************************************************************************/

#include<stdio.h>
int main(void)
{
	int chip_id = 0;
	int interrupt = 0;
	int reset = 0;
	chip_id = spi_test();
vsdvsd	interrupt = interrupt_test();
	reset = reset_test();
return 0;
}
