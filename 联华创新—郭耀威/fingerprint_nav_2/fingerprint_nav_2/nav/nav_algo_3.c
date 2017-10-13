/******************** (C) COPYRIGHT 2017 BTL ********************************
* File Name          : nav_algo.c
* Author               : guoyaowei
* Version              : 1.0
* Date                  : 2017.2.21
*******************************************************************************/
#include <string.h>

#include "nav.h"
#include "log.h"

#define TAG		"nav_algo"

typedef struct pos_info
{
 unsigned char x;
 unsigned char y;
}POS_INFO_T;
typedef struct edge_info
{
 POS_INFO_T left;
 POS_INFO_T right;
 POS_INFO_T up;
 POS_INFO_T down;
 unsigned char pixel_inds;
 unsigned char empty;
 POS_INFO_T	center;
}EDGE_INFO_T;

static unsigned long filter_time = 0, interval_time = 0;
static struct timeval start,stop, diff;
//static unsigned long diff;

static char gDirection = 0;

int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y)
{
	  int nsec;

	  if ( x->tv_sec>y->tv_sec )
				return -1;

	  if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) )
				return -1;

	  result->tv_sec = ( y->tv_sec-x->tv_sec );
	  result->tv_usec = ( y->tv_usec-x->tv_usec );

	  if (result->tv_usec<0)
	  {
				result->tv_sec--;
				result->tv_usec+=1000000;
	  }

	  return 0;
}

char cal_frame_info(unsigned char *buf, EDGE_INFO_T *info)
{
	unsigned int i = 0, j = 0, tmp = 0, sum = 0, sum_x = 0, sum_y = 0, sum_p = 0;

	for(i = 0, j = FINGER_WIDTH*(FINGER_HEIGHT-1) ; i < FINGER_WIDTH*(FINGER_HEIGHT-1); i++)
	{
		tmp = *(buf + i);
		if(tmp >= INACTIVE_PXL)
			j--;
		else
		{
			sum_p++;
			sum += tmp;
			sum_y += i / FINGER_WIDTH;
			sum_x += i % FINGER_WIDTH;
		}
	/*
		if((i%1000) == 1)
		  LOGD("pexl = %d\n",*(buf + i));
	*/
	}
	//LOGD(TAG, "ACTIVE_PXL num = %d\n", j);
	if(j < 200)
	{
		//LOGD(TAG, "ACTIVE_PXL = %d", j);
		info->empty = 1;
	}
	if(j == 0)
		j = 1;
	tmp = sum/j;
	info->pixel_inds = tmp + 1;
	info->center.x = sum_x / sum_p;
	info->center.y = sum_y / sum_p;
	LOGD(TAG, "sum_p = %d, info->center.x = %d, info->center.y = %d", sum_p, info->center.x, info->center.y);
	return tmp;

}

void cal_edg_pos(unsigned char *buf, EDGE_INFO_T *info)
{
	int i = 0, j = 0;

	for(i = 0; i < (FINGER_HEIGHT); i++)
	{
		for(j = 0; j < (FINGER_WIDTH); j++)
		{
			if(*(buf + i * FINGER_WIDTH+ j) <= info->pixel_inds * 2)
			{
				if(info->right.x != 0xff || info->right.y != 0xff)
				{
					info->left.x = i;
					info->left.y = j;
				}
				else
				{
					info->right.x = i;
					info->right.y = j;
				}
			}
		}
	}

	for(i = 1; i < (FINGER_WIDTH); i++)
	{

		for(j = 0; j < (FINGER_HEIGHT-1); j++)
		{
			if(*(buf + j * FINGER_WIDTH + i) <= info->pixel_inds * 2)
			{
				if(info->down.x != 0xff || info->down.y != 0xff)
				{
					info->up.x = j;
					info->up.y = i;
				}
				else
				{
					info->down.x = j;
					info->down.y = i;
				}
			}
		}
	}
}

void cal_move_dir_3182(EDGE_INFO_T *info_start, EDGE_INFO_T *info_end)
{
	int xy_sum_sq = 0, center_x_sq = 0, center_y_sq = 0;

	center_x_sq = info_end->center.x - info_start->center.x;
	center_y_sq = info_end->center.y - info_start->center.y;

	gDirection = ACT_NONE;
	
	LOGD(TAG,"center_x_sq + center_y_sq = %d, center_x_sq = %d, center_y_sq = %d!\n", center_x_sq + center_y_sq, center_x_sq, center_y_sq);

	if(center_y_sq >= 0)
	{
		if(center_y_sq > 40)	//10
		{
			LOGD(TAG, "up gDirection moving !\n");
			gDirection = DIR_RIGHT; 
		}
		else if(center_y_sq < -10)
		{
			LOGD(TAG, "right gDirection moving !\n");
			//gDirection = DIR_DOWN;		
			gDirection = ACT_NONE;
		}
	}
	else if(center_y_sq < 0)
	{
		if(center_y_sq > 10)
		{
			LOGD(TAG, "left gDirection moving !\n");
			//gDirection = DIR_UP;
			gDirection = ACT_NONE;

		}
		else if(center_y_sq < -40)		//30
		{
			LOGD(TAG, "down gDirection moving !\n");
			gDirection = DIR_LEFT; 

		}
	
	}
	
	if((interval_time > 120) && (interval_time < 500))//must be 120
	{
#ifndef NAV_DIR_EN
		LOGD(TAG, "CLK_SINGLE\n");
		gDirection = CLK_SINGLE;

#else
		if((abs(center_x_sq) + abs(center_y_sq) < 20)  && (abs(center_x_sq) <= 10 && abs(center_y_sq) <= 10))
		{
			LOGD(TAG, "CLK_SINGLE\n");
			gDirection = CLK_SINGLE;

		}
		else
			gDirection = gDirection;
			//gDirection = ACT_NONE;

#endif
	}
	if(gDirection == ACT_NONE)
	{
		LOGD(TAG, "NONE move !\n");
	}

}


void cal_move_dir_3290(EDGE_INFO_T *info_start, EDGE_INFO_T *info_end)
{
	int xy_sum_sq = 0, center_x_sq = 0, center_y_sq = 0;

	center_x_sq = info_end->center.x - info_start->center.x;
	center_y_sq = info_end->center.y - info_start->center.y;

	gDirection = ACT_NONE;
	
	LOGD(TAG,"abs(center_x_sq) - abs(center_y_sq) = %d, center_x_sq = %d, center_y_sq = %d!\n", abs(center_x_sq) - abs(center_y_sq), center_x_sq, center_y_sq);

	if(abs(center_x_sq) - abs(center_y_sq) >= 10)
	{
		if(center_x_sq > 20)	//10
		{
			LOGD(TAG, "right gDirection moving !\n");
			gDirection = DIR_RIGHT; 
		}
		else if(center_x_sq < -40)
		{
			LOGD(TAG, "left gDirection moving !\n");
			gDirection = DIR_LEFT;		

		}
	}
	else if(abs(center_x_sq) - abs(center_y_sq) < 10)
	{
		if(center_y_sq > 10)
		{
			LOGD(TAG, "up gDirection moving !\n");
			gDirection = DIR_UP;
		}
		else if(center_y_sq < -30)		//30
		{
			LOGD(TAG, "down gDirection moving !\n");
			gDirection = DIR_DOWN; 
		}
	
	}
	
	if((interval_time > 80) && (interval_time < 500))
	{

		if((abs(center_x_sq) + abs(center_y_sq) < 20)  && (abs(center_x_sq) <= 10 && abs(center_y_sq) <= 10))
		{
			LOGD(TAG, "CLK_SINGLE\n");
			gDirection = CLK_SINGLE;

		}
		else
			gDirection = gDirection;
			//gDirection = ACT_NONE;
	}
	if(gDirection == ACT_NONE)
	{
		LOGD(TAG, "NONE move !\n");
	}

}

char cal_touch_area(void)
{
	char sta1 = 0 ,sta2 = 0;
	//int i = 0;
	unsigned int frame = 1;
	char tmp = 0;
	unsigned char imageA[FINGER_WIDTH*FINGER_HEIGHT*2 + 1];

	gettimeofday(&start,0);
	EDGE_INFO_T edg_info_start, edg_info_end, edg_info_tmp;
	memset(&edg_info_start, 0xff, sizeof(EDGE_INFO_T));
	memset(&edg_info_end, 0xff, sizeof(EDGE_INFO_T));
	memset(&edg_info_tmp, 0xff, sizeof(EDGE_INFO_T));
	LOGD(TAG, "cal_touch_area \n");
	btl_nav_getImageData(imageA, 0);
	cal_frame_info(&imageA[0], &edg_info_start);
	cal_edg_pos(&imageA[0], &edg_info_start);
	cal_frame_info(&imageA[FINGER_WIDTH*FINGER_HEIGHT], &edg_info_end);
	cal_edg_pos(&imageA[FINGER_WIDTH*FINGER_HEIGHT], &edg_info_end);
	
	if(edg_info_start.empty == 1 || edg_info_end.empty == 1)
	{
		gDirection = ACT_NONE;
		return gDirection;
	}
	else if(imageA[FINGER_WIDTH*FINGER_HEIGHT*2] == 1 && edg_info_start.empty != 1 && edg_info_end.empty != 1)
	{
		gDirection = CLK_LONG_DOWN;
		return gDirection;
	}
	
	gettimeofday(&stop,0);
	timeval_subtract(&diff,&start,&stop);
	interval_time = diff.tv_sec * 1000 + diff.tv_usec/1000;
#ifdef CHIP_BF3182
	cal_move_dir_3182(&edg_info_start, &edg_info_end);
#else
	cal_move_dir_3290(&edg_info_start, &edg_info_end);
#endif

	
	LOGD(TAG, "gDirection = %d, interval_time = %ldms\n", gDirection, interval_time);


	return gDirection;
}
