/*
 * array.c
 *
 *  Created on: 2015年8月12日
 *      Author: work
 */

#define ARRAY_C_

#include <linux/types.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "array.h"
#include "buf_factory.h"
#include "fpga.h"
#include "sys_conf.h"
#include "position.h"
#include "message.h"
#include "msg_factory.h"
#include "mailbox.h"

struct list_head g_k1_sample_list;
struct list_head g_k2_sample_list;
static pthread_mutex_t mutex_k1_list;
static pthread_mutex_t mutex_k2_list;

__s32 g_pos_kick1_mbx_id = 0;
__s32 g_pos_kick2_mbx_id = 0;

__u32 cur_count_kt1 = 0;

struct buffer_factory *g_c1_sample_buf = NULL;

//static __u32 COMFIFOnum = 1;

__s32 search_count_array(__u32 count, __u32 pos)
{
	SampleArray* parray;
	struct Buffer *buf;
	SampleResult *psample;
	struct list_head *p;

	pthread_mutex_lock(&mutex_k1_list);
	p = g_k1_sample_list.next;     //将头节点的指针给予临时节点p
//	pthread_mutex_unlock(&mutex_k1_list);

	while(p != &g_k1_sample_list)
	{
		parray = (SampleArray*)(p->owner);

		if(parray->sample.count == count)
		{
//			printf("COM1FIFO set %ds\n",COMFIFOnum++);
//			printf("writeFIFO1 sample count %ds\n",parray->sample.count);
//			pthread_mutex_lock(&mutex_k1_list);
			list_del(p);
			buf_factory_recycle(0, parray->buf, g_c1_sample_buf);
			pthread_mutex_unlock(&mutex_k1_list);
			//根据结果进行剔除判断
			FPGA_WRITE16(fpga_base, FPGA_COM1FIFO_REG, 1);
			return 1;
		}
		p = p->next;
	}
	pthread_mutex_unlock(&mutex_k1_list);
	FPGA_WRITE16(fpga_base, FPGA_COM1FIFO_REG, 0);
	return -1;
}


__s32 search_wheelcode_array(__u64 wheel_code, __u32 pos)
{
	SampleArray* parray;
	struct Buffer *buf;
	SampleResult *psample;
	__u64 wheel_code_abs;

	struct list_head *p;

	if(pos == 1)
	{
		pthread_mutex_lock(&mutex_k1_list);
		p = g_k1_sample_list.next;     //将头节点的指针给予临时节点p
//		pthread_mutex_unlock(&mutex_k1_list);

		while(p != &g_k1_sample_list)
		{
			parray = (SampleArray*)(p->owner);


			if(g_sys_config.video2_valid)
			{
				wheel_code_abs = abs(wheel_code - parray->sample.wheel_code -
																		(g_sys_config.C1_KT1_delay - g_sys_config.C1_C2_delay));
			}
			else
			{
				wheel_code_abs = abs(wheel_code - parray->sample.wheel_code - g_sys_config.C1_KT1_delay);
			}

			if(wheel_code_abs <= g_sys_config.sample_length/2)
			{
//				printf("COM1FIFO set %d's\n",COMFIFOnum++);
//				printf("writeFIFO1 sample count %d\n",parray->sample.count);
//				pthread_mutex_lock(&mutex_k1_list);
				list_del(p);
				buf_factory_recycle(0, parray->buf, g_c1_sample_buf);
				pthread_mutex_unlock(&mutex_k1_list);
				//根据结果进行剔除判断
				FPGA_WRITE16(fpga_base, FPGA_COM1FIFO_REG, 1);
				return 1;
			}
			p = p->next;
		}
		pthread_mutex_unlock(&mutex_k1_list);
		FPGA_WRITE16(fpga_base, FPGA_COM1FIFO_REG, 0);
		return -1;
	}

	else
	{
		pthread_mutex_lock(&mutex_k2_list);
		p = g_k2_sample_list.next;     //将头节点的指针给予临时节点p
//		pthread_mutex_unlock(&mutex_k2_list);

		while(p != &g_k2_sample_list)
		{
			parray = (SampleArray*)(p->owner);

			if(g_sys_config.video2_valid)
			{
				wheel_code_abs = abs(wheel_code-parray->sample.wheel_code
																		- (g_sys_config.C1_KT1_delay - g_sys_config.C1_C2_delay)
																		- g_sys_config.KT1_KP1_delay
																		- g_sys_config.KP1_KT2_delay);
			}
			else
			{
				wheel_code_abs = abs(wheel_code-parray->sample.wheel_code
																		- g_sys_config.C1_KT1_delay
																		- g_sys_config.KT1_KP1_delay
																		- g_sys_config.KP1_KT2_delay);
			}

			if(wheel_code_abs <= g_sys_config.sample_length/2)
			{
//				printf("COM2FIFO set %d's\n",COMFIFOnum++);
//				printf("writeFIFO2 sample count %d\n",parray->sample.count);
//				pthread_mutex_lock(&mutex_k2_list);
				list_del(p);
				buf_factory_recycle(0, parray->buf, g_c1_sample_buf);
				pthread_mutex_unlock(&mutex_k2_list);
				//根据结果进行剔除判断
				FPGA_WRITE16(fpga_base, FPGA_COM2FIFO_REG, 1);
				return 1;
			}
			p = p->next;
		}
		pthread_mutex_unlock(&mutex_k2_list);
		FPGA_WRITE16(fpga_base, FPGA_COM2FIFO_REG, 0);
		return -1;
	}
}



void write_array(SampleResult sample)
{
	SampleArray* parray;
	struct Buffer *buf;
	if(sample.pos != 0)
	{
		buf = buf_factory_produce(g_c1_sample_buf);
		parray = (SampleArray* )(buf->memory);
		parray->buf = buf;
		memcpy(&(parray->sample), &sample, sizeof(SampleResult));

		if(sample.pos == 1)
		{
			pthread_mutex_lock(&mutex_k1_list);
			list_add_tail(&(parray->node), &g_k1_sample_list);
			pthread_mutex_unlock(&mutex_k1_list);
		}
		else if(sample.pos == 2)
		{
			pthread_mutex_lock(&mutex_k2_list);
			list_add_tail(&(parray->node), &g_k2_sample_list);
			pthread_mutex_unlock(&mutex_k2_list);
		}
		parray->node.owner = parray;
	}
}

void clear_array(void)
{
	struct list_head *p;
	SampleArray* parray;

	pthread_mutex_lock(&mutex_k1_list);
	p = g_k1_sample_list.next;     //将头节点的指针给予临时节点p
	while(p != &g_k1_sample_list)   //节点链表不为空，循环
	{
		parray = (SampleArray*)(p->owner);
		buf_factory_recycle(0, parray->buf, g_c1_sample_buf);
		p = p->next;
	}
	init_list_head(&g_k1_sample_list);
	pthread_mutex_unlock(&mutex_k1_list);

	pthread_mutex_lock(&mutex_k2_list);
	p = g_k2_sample_list.next;     //将头节点的指针给予临时节点p
	while(p != &g_k2_sample_list)   //节点链表不为空，循环
	{
		parray = (SampleArray*)(p->owner);
		buf_factory_recycle(0, parray->buf, g_c1_sample_buf);
		p = p->next;
	}
	init_list_head(&g_k2_sample_list);
	pthread_mutex_unlock(&mutex_k2_list);
}



void trig1_search_tsk(void)
{
	static	__u64 cur_wheel_code;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	msg_factory_cast(&msg, g_pos_kick1_mbx_id);
	while(1)
	{
		mailbox_pend(&msg);
		cur_count_kt1++;
		if(g_sys_config.kick_mode == 0)//内码盘模式
		{
			search_count_array(cur_count_kt1, 1);
		}
		else
		{
			data = msg.ops->get_data(&msg, &data_len);
			cur_wheel_code = *((__u64*)data);
			search_wheelcode_array(cur_wheel_code, 1);
		}
		msg_factory_recycle(&msg);
	}
}

void trig2_search_tsk(void)
{
	static	__u64 cur_wheel_code;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	msg_factory_cast(&msg, g_pos_kick2_mbx_id);
	while(1)
	{
		mailbox_pend(&msg);
		data = msg.ops->get_data(&msg, &data_len);
		cur_wheel_code = *((__u64*)data);
		//剔除工位二只能工作在外码盘模式下
		search_wheelcode_array(cur_wheel_code, 2);
		msg_factory_recycle(&msg);
	}
}


void array_buf_init(void)
{
	//创建样品信息队列缓冲
	create_buf_factory(128, 128, &g_c1_sample_buf);
	init_list_head(&g_k1_sample_list);
	init_list_head(&g_k2_sample_list);
	g_pos_kick1_mbx_id  = mailbox_create("/pos_c1");
	g_pos_kick2_mbx_id  = mailbox_create("/pos_c2");
	pthread_mutex_init(&mutex_k1_list,NULL);
	pthread_mutex_init(&mutex_k2_list,NULL);
	add_new_thread(NULL, (void *)&trig1_search_tsk, 12, 0, 16*1024);
	add_new_thread(NULL, (void *)&trig2_search_tsk, 12, 0, 16*1024);
}

