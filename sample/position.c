/*
 * pos_report.c
 *
 *  Created on: 2015年8月12日
 *      Author: work
 */

#define POSITION_C_

#include <semaphore.h>
#include <linux/types.h>
#include <string.h>

#include "local_protocol.h"
#include "local_socket.h"
#include "message.h"
#include "mailbox.h"
#include "msg_factory.h"
#include "thread.h"
#include "buf_factory.h"
#include "position.h"
#include "debug.h"
#include "list.h"
#include "fpga.h"

sem_t gsem_upload_pos;
__u32 upload_pos_select;

__s32 g_pos_kt1_mbx_id = 0;
__s32 g_pos_kt2_mbx_id = 0;
__s32 g_pos_kp1_mbx_id = 0;
__s32 g_pos_kp2_mbx_id = 0;
__s32 g_pos_kft1_mbx_id = 0;
__s32 g_pos_kft2_mbx_id = 0;


struct list_head g_c1_kt1_list;
struct list_head g_kp1_kft1_list;
struct list_head g_c1_kt2_list;
struct list_head g_kp2_kft2_list;

struct buffer_factory *g_c1_kt1_buf;
struct buffer_factory *g_kp1_kft1_buf;
struct buffer_factory *g_c1_kt2_buf;
struct buffer_factory *g_kp2_kft2_buf;

pthread_mutex_t mutex_c1_kt1_list;
pthread_mutex_t mutex_c1_kt2_list;

pthread_mutex_t mutex_kp1_kft1_list;
pthread_mutex_t mutex_kp2_kft2_list;


/*位置报告发送*/
static struct LocalInfoHead pos_report_head =
{
		.type = 0xAACC0204,
		.length = 12
};

//相机1到剔除对管1的距离
void dis_c1_kt1_tsk()
{
	static __u32 dis,item = 2;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	__u64 cur_wheel_code;
	struct list_head *plist;
	struct message *msg2main;
	PositionArray *parray;
	msg_factory_cast(&msg, g_pos_kt1_mbx_id);
	while(1)
	{
		//填充计算距离代码
		mailbox_pend(&msg);
		if(list_empty(&g_c1_kt1_list))
		{
			MSG("g_c1_kt1_list NULL");
		}
		else
		{
			plist = g_c1_kt1_list.next;
			parray = (PositionArray*)(plist->owner);

			data = msg.ops->get_data(&msg, &data_len);
			cur_wheel_code = *((__u64*)data);
			dis = cur_wheel_code-parray->sample.wheel_code;
			pthread_mutex_lock(&mutex_c1_kt1_list);
			list_del(plist);
			pthread_mutex_unlock(&mutex_c1_kt1_list);
			buf_factory_recycle(0, parray->buf, g_c1_kt1_buf);
		}
		msg_factory_recycle(&msg);

		msg2main = msg_factory_produce(g_local_mbx_id);
		msg2main->ops->set_data(msg2main, &pos_report_head, sizeof(pos_report_head), NULL, 0);
		msg2main->ops->set_data(msg2main, &item, 4, NULL, 0);
		msg2main->ops->set_data(msg2main, &dis, 4, NULL, 0);
		mailbox_post(msg2main);
	}
}

//剔除工位1至剔除反馈对管1延时
void dis_kp1_kft1_tsk()
{
	static __u32 dis,item = 3;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	__u64 cur_wheel_code;
	static 	struct timeval cur_time;
	__u32 ConvertCalcTime;
	struct list_head *plist;

	struct message *msg2main;
	PositionArray *parray;
	msg_factory_cast(&msg, g_pos_kft1_mbx_id);
	while(1)
	{
		//填充计算距离代码
		mailbox_pend(&msg);
		if(list_empty(&g_kp1_kft1_list))
		{
			MSG("g_kp1_kft1_list NULL");
		}
		else
		{
			plist = g_kp1_kft1_list.next;
			parray = (PositionArray*)(plist->owner);
			data = msg.ops->get_data(&msg, &data_len);
//			cur_wheel_code = *((__u64*)data);
//			dis = cur_wheel_code - parray->sample.wheel_code;
			memcpy(&cur_time, data, data_len);
			ConvertCalcTime = 1000000 * (cur_time.tv_sec - parray->sample.c_time.tv_sec)
									+ (cur_time.tv_usec -  parray->sample.c_time.tv_usec);
			dis = ConvertCalcTime/50;
			pthread_mutex_lock(&mutex_kp1_kft1_list);
			list_del(plist);
			pthread_mutex_unlock(&mutex_kp1_kft1_list);
			buf_factory_recycle(0, parray->buf, g_kp1_kft1_buf);
		}
		msg_factory_recycle(&msg);

		msg2main = msg_factory_produce(g_local_mbx_id);
		msg2main->ops->set_data(msg2main, &pos_report_head, sizeof(pos_report_head), NULL, 0);
		msg2main->ops->set_data(msg2main, &item, 4, NULL, 0);
		msg2main->ops->set_data(msg2main, &dis, 4, NULL, 0);
		mailbox_post(msg2main);
	}
}

//相机1至剔除对管2距离
void dis_c1_kt2_tsk()
{
	static __u32 dis,item = 4;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	__u64 cur_wheel_code;
	struct list_head *plist;
	struct message *msg2main;
	PositionArray *parray;
	msg_factory_cast(&msg, g_pos_kt2_mbx_id);
	while(1)
	{
		//填充计算距离代码
		mailbox_pend(&msg);
		if(list_empty(&g_c1_kt2_list))
		{
			MSG("g_c1_kt2_list NULL");
		}
		else
		{
			plist = g_c1_kt2_list.next;
			parray = (PositionArray*)(plist->owner);

			data = msg.ops->get_data(&msg, &data_len);
			cur_wheel_code = *((__u64*)data);
			dis = cur_wheel_code-parray->sample.wheel_code;
			pthread_mutex_lock(&mutex_c1_kt2_list);
			list_del(plist);
			pthread_mutex_unlock(&mutex_c1_kt2_list);
			buf_factory_recycle(0, parray->buf, g_c1_kt2_buf);
		}
		msg_factory_recycle(&msg);

		msg2main = msg_factory_produce(g_local_mbx_id);
		msg2main->ops->set_data(msg2main, &pos_report_head, sizeof(pos_report_head), NULL, 0);
		msg2main->ops->set_data(msg2main, &item, 4, NULL, 0);
		msg2main->ops->set_data(msg2main, &dis, 4, NULL, 0);
		mailbox_post(msg2main);
	}
}

//剔除工位2至剔除反馈对管2延时
void dis_kp2_kft2_tsk()
{
	static __u32 dis,item = 5;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	__u64 cur_wheel_code;
	struct list_head *plist;
	static 	struct timeval cur_time;
	__u32 ConvertCalcTime;
	struct message *msg2main;
	PositionArray *parray;
	msg_factory_cast(&msg, g_pos_kft2_mbx_id);
	while(1)
	{
		//填充计算距离代码
		mailbox_pend(&msg);
		if(list_empty(&g_kp2_kft2_list))
		{
			MSG("g_kp2_kft2_list NULL");
		}
		else
		{
			plist = g_kp2_kft2_list.next;
			parray = (PositionArray*)(plist->owner);

			data = msg.ops->get_data(&msg, &data_len);
//			cur_wheel_code = *((__u64*)data);
//			dis = cur_wheel_code-parray->sample.wheel_code;
			memcpy(&cur_time, data, data_len);
			ConvertCalcTime = 1000000 * (cur_time.tv_sec - parray->sample.c_time.tv_sec)
									+ (cur_time.tv_usec -  parray->sample.c_time.tv_usec);
			dis = ConvertCalcTime/50;
//			printf("ConvertCalcTime = %d\n",ConvertCalcTime);
			pthread_mutex_lock(&mutex_kp2_kft2_list);
			list_del(plist);
			pthread_mutex_unlock(&mutex_kp2_kft2_list);
			buf_factory_recycle(0, parray->buf, g_kp2_kft2_buf);
		}
		msg_factory_recycle(&msg);

		msg2main = msg_factory_produce(g_local_mbx_id);
		msg2main->ops->set_data(msg2main, &pos_report_head, sizeof(pos_report_head), NULL, 0);
		msg2main->ops->set_data(msg2main, &item, 4, NULL, 0);
		msg2main->ops->set_data(msg2main, &dis, 4, NULL, 0);

		mailbox_post(msg2main);
	}
}



void write_kp1_array_tsk(void)
{
	struct Buffer *buf;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	PositionArray* parray;
	static __u32 kp1_sample_count = 0;

	msg_factory_cast(&msg, g_pos_kp1_mbx_id);
	while(1)
	{
		//填充计算距离代码
		mailbox_pend(&msg);
		buf = buf_factory_produce(g_kp1_kft1_buf);
		parray = (PositionArray* )(buf->memory);
		parray->buf = buf;
		kp1_sample_count++;
		parray->sample.count = kp1_sample_count;
		data = msg.ops->get_data(&msg, &data_len);
//		parray->sample.wheel_code = *((__u64*)data);
		memcpy(&(parray->sample.c_time), data, data_len);
		pthread_mutex_lock(&mutex_kp1_kft1_list);
		list_add_tail(&(parray->node), &g_kp1_kft1_list);
		parray->node.owner = parray;
		pthread_mutex_unlock(&mutex_kp1_kft1_list);
		msg_factory_recycle(&msg);
	}
}

void write_kp2_array_tsk(void)
{
	struct Buffer *buf;
	struct message msg;
	void *data = NULL;
	__u32 data_len = 0;
	PositionArray* parray;
	static __u32 kp2_sample_count = 0;

	msg_factory_cast(&msg, g_pos_kp2_mbx_id);
	while(1)
	{
		//填充计算距离代码
		mailbox_pend(&msg);
		buf = buf_factory_produce(g_kp2_kft2_buf);
		parray = (PositionArray* )(buf->memory);
		parray->buf = buf;
		kp2_sample_count++;
		parray->sample.count = kp2_sample_count;
		data = msg.ops->get_data(&msg, &data_len);
//		parray->sample.wheel_code = *((__u64*)data);
		memcpy(&(parray->sample.c_time), data, data_len);
		pthread_mutex_lock(&mutex_kp2_kft2_list);
		list_add_tail(&(parray->node), &g_kp2_kft2_list);
		parray->node.owner = parray;
		pthread_mutex_unlock(&mutex_kp2_kft2_list);
		msg_factory_recycle(&msg);
	}
}




void position_context_init(void)
{

	g_pos_kt1_mbx_id  = mailbox_create("/pos_kt1");
	g_pos_kt2_mbx_id  = mailbox_create("/pos_kt2");
	g_pos_kp1_mbx_id  = mailbox_create("/pos_kp1");
	g_pos_kp2_mbx_id  = mailbox_create("/pos_kp2");
	g_pos_kft1_mbx_id = mailbox_create("/pos_kft1");
	g_pos_kft2_mbx_id = mailbox_create("/pos_kft2");


	init_list_head(&g_c1_kt1_list);
	init_list_head(&g_kp1_kft1_list);
	init_list_head(&g_c1_kt2_list);
	init_list_head(&g_kp2_kft2_list);

	create_buf_factory(128, 128, &g_c1_kt1_buf);
	create_buf_factory(128, 128, &g_kp1_kft1_buf);
	create_buf_factory(128, 128, &g_c1_kt2_buf);
	create_buf_factory(128, 128, &g_kp2_kft2_buf);

	pthread_mutex_init(&mutex_c1_kt1_list,NULL);
	pthread_mutex_init(&mutex_c1_kt2_list,NULL);

	add_new_thread(NULL, (void *)&dis_c1_kt1_tsk, 8, 0, 8*1024);
	add_new_thread(NULL, (void *)&dis_kp1_kft1_tsk, 8, 0, 8*1024);
	add_new_thread(NULL, (void *)&dis_c1_kt2_tsk, 8, 0, 8*1024);
	add_new_thread(NULL, (void *)&dis_kp2_kft2_tsk, 8, 0, 8*1024);
	add_new_thread(NULL, (void *)&write_kp1_array_tsk, 9, 0, 8*1024);
	add_new_thread(NULL, (void *)&write_kp2_array_tsk, 9, 0, 8*1024);
}




