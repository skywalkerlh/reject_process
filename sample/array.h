/*
 * array.h
 *
 *  Created on: 2015年8月13日
 *      Author: work
 */

#ifndef ARRAY_H_
#define ARRAY_H_
#include <semaphore.h>
#include "list.h"

#pragma pack(1)
typedef struct SampleResult
{
	__u8  result[8];		//算法结果
	__u32 count;				//样品计数
	__u64 wheel_code;	//拍摄时刻码盘
	__u32 pos;						//待剔除工位
	__u32 alarm;				//报警
}SampleResult;
#pragma pack()

typedef struct SampleArray
{
	SampleResult sample;		//算法结果
	struct Buffer *buf;
	struct list_head node;
}SampleArray;

#ifndef ARRAY_C_

//extern sem_t gsem_kick1;
//extern sem_t gsem_kick2;
extern __s32 g_pos_kick1_mbx_id;
extern __s32 g_pos_kick2_mbx_id;

#endif

void array_buf_init(void);

void write_array(SampleResult sample);
void clear_array(void);

#endif /* ARRAY_H_ */
