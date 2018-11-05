/*
 * position.h
 *
 *  Created on: 2015年9月8日
 *      Author: work
 */

#ifndef POSITION_H_
#define POSITION_H_

#include <semaphore.h>
#include <time.h>
#include "list.h"

typedef struct SamplePosition
{
	__u32 count;				//样品计数
	__u64 wheel_code;	//写入阵列时刻刻码盘
	struct timeval c_time;
}SamplePosition;

typedef struct PositionArray
{
	SamplePosition sample;		//算法结果
	struct Buffer *buf;
	struct list_head node;
}PositionArray;


#ifndef POSITION_C_

extern __s32 g_pos_kt1_mbx_id;
extern __s32 g_pos_kt2_mbx_id;
extern __s32 g_pos_kp1_mbx_id;
extern __s32 g_pos_kp2_mbx_id;
extern __s32 g_pos_kft1_mbx_id;
extern __s32 g_pos_kft2_mbx_id;


extern struct list_head g_c1_kt1_list;
extern struct list_head g_kp1_kft1_list;
extern struct list_head g_c1_kt2_list;
extern struct list_head g_kp2_kft2_list;

extern struct buffer_factory *g_c1_kt1_buf;
extern struct buffer_factory *g_kp1_kft1_buf;
extern struct buffer_factory *g_c1_kt2_buf;
extern struct buffer_factory *g_kp2_kft2_buf;

extern pthread_mutex_t mutex_c1_kt1_list;
extern pthread_mutex_t mutex_c1_kt2_list;
#endif


void position_context_init(void);

#endif /* POSITION_H_ */
