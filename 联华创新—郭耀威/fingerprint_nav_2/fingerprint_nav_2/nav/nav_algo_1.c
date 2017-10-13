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
	unsigned int i = 0, j = 0, tmp = 0, sum = 0;

	for(i = 0, j = 112 * 96 ; i < 112 * 96; i++)
	{
		tmp = *(buf + i);
		if(tmp >= INACTIVE_PXL)
			j--;
		else
		{
			sum += tmp;
		}
	/*
		if((i%1000) == 1)
		  LOGD("pexl = %d\n",*(buf + i));
	*/
	}
	//LOGD(TAG, "ACTIVE_PXL num = %d\n", j);
	if(j < 200)
	{
		LOGD(TAG, "ACTIVE_PXL = %d", j);
		info->empty = 1;
	}
	if(j == 0)
		j = 1;
	tmp = sum/j;
	info->pixel_inds = tmp + 1;

	return tmp;

}

void cal_edg_pos(unsigned char *buf, EDGE_INFO_T *info)
{
	int i = 0, j = 0;

	for(i = 0; i < (96); i++)
	{
		for(j = 0; j < (112); j++)
		{
			if(*(buf + i * 112 + j) <= info->pixel_inds * 2)
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

	for(i = 1; i < (112); i++)
	{

		for(j = 0; j < (96-1); j++)
		{
/*			if(i == 0)
			{
				LOGD(TAG, "buf i=0 y = %d : %d", j, *(buf + j * 112 + i));
				usleep(200);
				continue;
			}
			*/
			if(*(buf + j * 112 + i) <= info->pixel_inds * 2)
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

void cal_move_dir(EDGE_INFO_T *info_start, EDGE_INFO_T *info_end)
{
	//static char reamain = 0;
	int tmp = 0;
	int right_x_df = 0, right_y_df = 0,
				  left_x_df = 0,  left_y_df = 0,
		          down_x_df = 0,  down_y_df = 0,
		          up_x_df = 0, 	up_y_df = 0;

	unsigned char right_x_sq = 0, right_y_sq = 0,
				  left_x_sq = 0,  left_y_sq = 0,
		          down_x_sq = 0,  down_y_sq = 0,
		          up_x_sq = 0, 	up_y_sq = 0;
	int xy_sum_sq = 0;
	char x_neg = 0, x_pos = 0, y_neg = 0, y_pos = 0;


	right_x_df = info_end->right.x - info_start->right.x;
	right_y_df = info_end->right.y - info_start->right.y;
	left_x_df = info_end->left.x - info_start->left.x;
	left_y_df = info_end->left.y - info_start->left.y;

	down_x_df = info_end->down.x - info_start->down.x;
	down_y_df = info_end->down.y - info_start->down.y;
	up_x_df = info_end->up.x - info_start->up.x;
	up_y_df = info_end->up.y - info_start->up.y;

	right_x_sq = abs(info_start->right.x - info_end->right.x);
	right_y_sq = abs(info_start->right.y - info_end->right.y);
	left_x_sq = abs(info_start->left.x - info_end->left.x);
	left_y_sq = abs(info_start->left.y - info_end->left.y);

	down_x_sq = abs(info_start->down.x - info_end->down.x);
	down_y_sq = abs(info_start->down.y - info_end->down.y);
	up_x_sq = abs(info_start->up.x - info_end->up.x);
	up_y_sq = abs(info_start->up.y - info_end->up.y);

	right_x_df > 0 ? x_pos++ : x_neg++;
	right_x_df > 0 ? x_pos++ : x_neg++;
	up_x_df > 0 ? x_pos++ : x_neg++;
	down_x_df > 0 ? x_pos++ : x_neg++;

	right_y_df > 0 ? y_pos++ : y_neg++;
	right_y_df > 0 ? y_pos++ : y_neg++;
	up_y_df > 0 ? y_pos++ : y_neg++;
	down_y_df > 0 ? y_pos++ : y_neg++;

	xy_sum_sq = abs((right_x_sq + left_x_sq + down_x_sq + up_x_sq) - (right_y_sq + left_y_sq + down_y_sq + up_y_sq));

	LOGD(TAG, "x_neg = %d, x_pos = %d, y_neg = %d, y_pos = %d \n", x_neg, x_pos, y_neg, y_pos);
	LOGD(TAG, "right_x_df = %d, left_x_df = %d, up_x_df = %d, down_x_df = %d \n", right_x_df, left_x_df, up_x_df, down_x_df);
	LOGD(TAG, "right_y_df = %d, left_y_df = %d, up_y_df = %d, down_y_df = %d \n", right_y_df, left_y_df, up_y_df, down_y_df);

	LOGD(TAG, "info_start->right.x = %d, info_end->right.x = %d\n", info_start->right.x, info_end->right.x);
	LOGD(TAG, "info_start->left.x  = %d, info_end->left.x  = %d\n", info_start->left.x, info_end->left.x);

	LOGD(TAG, "info_start->up.x    = %d, info_end->up.x    = %d\n", info_start->up.x, info_end->up.x);
	LOGD(TAG, "info_start->down.x  = %d, info_end->down.x  = %d\n", info_start->down.x, info_end->down.x);

	LOGD(TAG, "info_start->right.y = %d, info_end->right.y = %d\n", info_start->right.y, info_end->right.y);
	LOGD(TAG, "info_start->left.y  = %d, info_end->left.y  = %d\n", info_start->left.y, info_end->left.y);

	LOGD(TAG, "info_start->up.y    = %d, info_end->up.y    = %d\n", info_start->up.y, info_end->up.y);
	LOGD(TAG, "info_start->down.y  = %d, info_end->down.y  = %d\n", info_start->down.y, info_end->down.y);
/*
	LOGD("right_x_sq = %d\n", right_x_sq);
	LOGD("right_y_sq = %d\n", right_y_sq);
	LOGD("left_x_sq = %d\n", left_x_sq);
	LOGD("left_y_sq = %d\n", left_y_sq);
	LOGD("down_x_sq = %d\n", down_x_sq);
	LOGD("down_y_sq = %d\n", down_y_sq);
	LOGD("up_x_sq = %d\n", up_x_sq);
	LOGD("up_y_sq = %d\n", up_y_sq);
*/

	gDirection = ACT_NONE;

	if(((right_x_sq > 96) || (left_x_sq > 96) || (down_x_sq > 96) || (up_x_sq > 96)) || ((right_y_sq > 112) || (left_y_sq > 112) || (down_y_sq > 112) || (up_y_sq > 112)))
	{
		gDirection = ACT_NONE;
		return;
	}
	LOGD(TAG,"x - y = %d!\n", (right_x_sq + left_x_sq + down_x_sq + up_x_sq) - (right_y_sq + left_y_sq + down_y_sq + up_y_sq));
	if(((right_x_sq + left_x_sq + down_x_sq + up_x_sq) >= (right_y_sq + left_y_sq + down_y_sq + up_y_sq)) && (((right_x_sq >= 10) || (left_x_sq >= 10)) && ((down_x_sq >= 5) || (up_x_sq >= 5))))
	{
		if(info_start->right.x <= info_end->right.x && info_start->left.x <= info_end->left.x)
		{
			LOGD(TAG,"right gDirection moving !\n");
			gDirection = DIR_RIGHT;		//right
		}
		else if(info_start->right.x >= info_end->right.x && info_start->left.x >= info_end->left.x)
		{
			LOGD(TAG,"left gDirection moving !\n");
			gDirection = DIR_LEFT;		//left
		}
		else
			LOGD(TAG,"x gDirection move invalid !\n");
	}
	else if(((right_x_sq + left_x_sq + down_x_sq + up_x_sq) < (right_y_sq + left_y_sq + down_y_sq + up_y_sq)) && (((right_y_sq >= 15) || (left_y_sq >= 15)) &&  ((down_y_sq >= 40) || (up_y_sq >= 40))))
	{
		if(info_start->down.y <= info_end->down.y && info_start->up.y <= info_end->up.y)
		{
			LOGD(TAG,"down gDirection moving !\n");
			gDirection = DIR_DOWN;		//down
		}
		else if(info_start->down.y >= info_end->down.y && info_start->up.y >= info_end->up.y)
		{
			LOGD(TAG,"up gDirection moving !\n");
			gDirection = DIR_UP;		//up
		}
		else
			LOGD(TAG,"y gDirection move invalid !\n");
	}

	if(gDirection != ACT_NONE)
	{

	}

	else if((interval_time > 180) && (interval_time < 500))
	{
		LOGD(TAG, "CLK_SINGLE judge !\n");
		right_x_sq > 30 ? tmp++ : tmp;
		left_x_sq > 30 ? tmp++ : tmp;
		down_x_sq > 30 ? tmp++ : tmp;
		up_x_sq > 30 ? tmp++ : tmp;

		right_y_sq > 30 ? tmp++ : tmp;
		left_y_sq > 30 ? tmp++ : tmp;
		down_y_sq > 30 ? tmp++ : tmp;
		up_y_sq > 30 ? tmp++ : tmp;

		if(tmp <= 3)
		{
			gDirection = CLK_SINGLE;

		}
		else
			gDirection = ACT_NONE;
	}
/*
	else if((interval_time * MS_ONE) > 1000)
	{
		LOGD("CLK_LONG judge !\n");
		right_x_sq > 50 ? tmp++ : tmp;
		left_x_sq > 50 ? tmp++ : tmp;
		down_x_sq > 50 ? tmp++ : tmp;
		up_x_sq > 50 ? tmp++ : tmp;

		right_y_sq > 50 ? tmp++ : tmp;
		left_y_sq > 50 ? tmp++ : tmp;
		down_y_sq > 50 ? tmp++ : tmp;
		up_y_sq > 50 ? tmp++ : tmp;

		if(tmp <= 3)
			gDirection = CLK_LONG;
		else
			gDirection = ACT_NONE;
	}
*/
	else if(((right_x_sq > 30) || (left_x_sq > 30) || (down_x_sq > 30) || (up_x_sq > 30)) || ((right_y_sq > 25) || (left_y_sq > 25) || (down_y_sq > 25) || (up_y_sq > 25)))
	{

		LOGD(TAG, "remain !\n");
/*
		reamain++;
		reamain = reamain % 3;
		if(reamain == 1)
			return;
		else
*/
			gDirection = ACT_NONE;
	}
	else
	{
	LOGD(TAG, "NONE move !\n");
		gDirection = ACT_NONE;
	}
}

char cal_touch_area(void)
{
	char sta1 = 0 ,sta2 = 0;
	//int i = 0;
	unsigned int frame = 1;
	char tmp = 0;
	unsigned char imageA[112*96*2 + 1];

	gettimeofday(&start,0);
	EDGE_INFO_T edg_info_start, edg_info_end, edg_info_tmp;
	memset(&edg_info_start, 0xff, sizeof(EDGE_INFO_T));
	memset(&edg_info_end, 0xff, sizeof(EDGE_INFO_T));
	memset(&edg_info_tmp, 0xff, sizeof(EDGE_INFO_T));
	LOGD(TAG, "cal_touch_area \n");
	btl_nav_getImageData(imageA, 170);
	if(imageA[112*96*2] == 1)
	{
		gDirection = CLK_LONG_DOWN;
		return gDirection;
	}
	cal_frame_info(&imageA[0], &edg_info_start);
	cal_edg_pos(&imageA[0], &edg_info_start);
	cal_frame_info(&imageA[112*96], &edg_info_end);
	cal_edg_pos(&imageA[112*96], &edg_info_end);

	while(0)
	{

		btl_nav_getImageData(imageA, 170);
/*
		if(diff.tv_sec >= 1)
		{
			LOGD("frame = %d\n", frame);
			break;
		}
		else
		{
			btl_core_setReadNav(fd);
			continue;
		}
*/
		if(frame == 1)
		{
			cal_frame_info(imageA, &edg_info_start);

			if(edg_info_start.empty == 1)
			{
				sta1 = -1;
				break;
			}

			cal_edg_pos(imageA, &edg_info_start);
			/*
			LOGD("edg_info_start: \n");
			LOGD("left.x  = %d, left.y  = %d\n", edg_info_start.left.x, edg_info_start.left.y);
			LOGD("right.x = %d, right.y = %d\n", edg_info_start.right.x, edg_info_start.right.y);
			LOGD("down.x  = %d, down.y  = %d\n", edg_info_start.down.x, edg_info_start.down.y);
			LOGD("up.x    = %d, up.y    = %d\n", edg_info_start.up.x, edg_info_start.up.y);
			LOGD("edg_info_start.pixel_inds %d, edg_info_start.empty = %d\n",edg_info_start.pixel_inds, edg_info_start.empty);
			*/
		}
		else
		{
			memset(&edg_info_tmp, 0xff, sizeof(EDGE_INFO_T));
			cal_frame_info(imageA, &edg_info_tmp);

			if(edg_info_tmp.empty == 1)
			{
				if(frame == 2)
				{
					sta1 = -2;
					break;
				}
				else
				{
					sta1 = 1;
					break;
				}
			}

			memset(&edg_info_end, 0xff, sizeof(EDGE_INFO_T));
	/*
			LOGD("sizeof(EDGE_INFO_T) = %ld\n", sizeof(EDGE_INFO_T));
			LOGD("edg_info_tmp: \n");
			LOGD("left.x  = %d, left.y  = %d\n", edg_info_tmp.left.x, edg_info_tmp.left.y);
			LOGD("right.x = %d, right.y = %d\n", edg_info_tmp.right.x, edg_info_tmp.right.y);
			LOGD("down.x  = %d, down.y  = %d\n", edg_info_tmp.down.x, edg_info_tmp.down.y);
			LOGD("up.x    = %d, up.y    = %d\n", edg_info_tmp.up.x, edg_info_tmp.up.y);
			LOGD("edg_info_tmp.pixel_inds %d, edg_info_tmp.empty = %d\n",edg_info_tmp.pixel_inds, edg_info_tmp.empty);
	*/
			//memcpy(&edg_info_end, &edg_info_tmp, sizeof(EDGE_INFO_T));
			edg_info_end.empty = edg_info_tmp.empty;
			edg_info_end.pixel_inds = edg_info_tmp.pixel_inds;
	/*
			LOGD("edg_info_end: \n");
			LOGD("left.x  = %d, left.y  = %d\n", edg_info_end.left.x, edg_info_end.left.y);
			LOGD("right.x = %d, right.y = %d\n", edg_info_end.right.x, edg_info_end.right.y);
			LOGD("down.x  = %d, down.y  = %d\n", edg_info_end.down.x, edg_info_end.down.y);
			LOGD("up.x    = %d, up.y    = %d\n", edg_info_end.up.x, edg_info_end.up.y);
			LOGD("edg_info_end.pixel_inds %d, edg_info_end.empty = %d\n",edg_info_end.pixel_inds, edg_info_end.empty);
	*/
			cal_edg_pos(imageA, &edg_info_end);
	/*
			LOGD("after cal edg_info_end: \n ");
			LOGD("left.x  = %d, left.y  = %d\n", edg_info_end.left.x, edg_info_end.left.y);
			LOGD("right.x = %d, right.y = %d\n", edg_info_end.right.x, edg_info_end.right.y);
			LOGD("down.x  = %d, down.y  = %d\n", edg_info_end.down.x, edg_info_end.down.y);
			LOGD("up.x    = %d, up.y    = %d\n", edg_info_end.up.x, edg_info_end.up.y);
			LOGD("edg_info_end.pixel_inds %d, edg_info_end.empty = %d\n",edg_info_end.pixel_inds, edg_info_end.empty);
	*/
	/*
			for(i = 0; i < 20; i++)
			{
				LOGD("piex = %d", *(buf + 112*40 + i));
			}
			printk("\n");
			if(frame == 10)
			{
				gDirection =3;
				sta = 1;
				break;
			}
	*/
		}
		frame++;
		//btl_core_setReadNav(fd);

		gettimeofday(&stop,0);
		timeval_subtract(&diff,&start,&stop);
		interval_time = diff.tv_sec * 1000 + diff.tv_usec/1000;
		if(sta2 == 0)
		{
			abs(edg_info_start.right.x - edg_info_end.right.x) > 30 ? tmp++ : tmp;
			abs(edg_info_start.right.y - edg_info_end.right.y) > 30 ? tmp++ : tmp;
			abs(edg_info_start.left.x - edg_info_end.left.x) > 30 ? tmp++ : tmp;
			abs(edg_info_start.left.y - edg_info_end.left.y) > 30 ? tmp++ : tmp;

			abs(edg_info_start.down.x - edg_info_end.down.x) > 30 ? tmp++ : tmp;
			abs(edg_info_start.down.y - edg_info_end.down.y) > 30 ? tmp++ : tmp;
			abs(edg_info_start.up.x - edg_info_end.up.x) > 30 ? tmp++ : tmp;
			abs(edg_info_start.up.y - edg_info_end.up.y) > 30 ? tmp++ : tmp;

			if(interval_time > 800 && tmp <= 3)
			{
				gDirection = CLK_LONG_DOWN;
				//btl_core_reportNavKey(fd, CLK_LONG_DOWN, 0);
				sta2 = 1;
			}else
				tmp = 0;
		}

		usleep(10000);
	}
	gettimeofday(&stop,0);
	timeval_subtract(&diff,&start,&stop);
	interval_time = diff.tv_sec * 1000 + diff.tv_usec/1000;
	if(1)
	//if(sta2 == 0 && sta1 > 0)
	{

		//if(interval_time > (10 * MS_TEN))
		{
			LOGD(TAG, "edg_info_start: \n");
			LOGD(TAG, "left.x  = %d, left.y  = %d\n", edg_info_start.left.x, edg_info_start.left.y);
			LOGD(TAG, "right.x = %d, right.y = %d\n", edg_info_start.right.x, edg_info_start.right.y);
			LOGD(TAG, "down.x  = %d, down.y  = %d\n", edg_info_start.down.x, edg_info_start.down.y);
			LOGD(TAG, "up.x    = %d, up.y    = %d\n", edg_info_start.up.x, edg_info_start.up.y);
			LOGD(TAG, "edg_info_start.pixel_inds %d, edg_info_start.empty = %d\n",edg_info_start.pixel_inds, edg_info_start.empty);

			LOGD(TAG, "edg_info_end: \n");
			LOGD(TAG, "left.x  = %d, left.y  = %d\n", edg_info_end.left.x, edg_info_end.left.y);
			LOGD(TAG, "right.x = %d, right.y = %d\n", edg_info_end.right.x, edg_info_end.right.y);
			LOGD(TAG, "down.x  = %d, down.y  = %d\n", edg_info_end.down.x, edg_info_end.down.y);
			LOGD(TAG, "up.x    = %d, up.y    = %d\n", edg_info_end.up.x, edg_info_end.up.y);
			LOGD(TAG, "edg_info_end.pixel_inds %d, edg_info_end.empty = %d\n",edg_info_end.pixel_inds, edg_info_end.empty);


			cal_move_dir(&edg_info_start, &edg_info_end);
			LOGD(TAG, "gDirection = %d, interval_time = %ldms\n", gDirection, interval_time);
		}
	}
	else if(sta2 == 1)
	{
		gDirection = CLK_LONG_UP;
	}

	return gDirection;
}
