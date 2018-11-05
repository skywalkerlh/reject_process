/*
 * local_protocol.c
 *
 *  Created on: 2015年8月14日
 *      Author: work
 */

#include <stdio.h>
#include <string.h>

#include "local_socket.h"
#include "array.h"
#include "fpga.h"
#include "sys_conf.h"
#include "position.h"
#include "buf_factory.h"
#include "local_protocol.h"
#include "msg_factory.h"
#include "mailbox.h"

extern __u32 cur_count_kt1;
__u32 g_sys_reset_flag = 0;


/*握手版本发送*/
static struct LocalInfoHead handshake_head =
{
		.type = 0xAACC0200,
		.length = 4 + 32
};

static __u8 version[32];

void recv_handshake_cmd()
{
	struct message *msg2main;

	strcpy(version,BUILD_DATE);

	msg2main = msg_factory_produce(g_local_mbx_id);
	msg2main->ops->set_data(msg2main, &handshake_head, sizeof(handshake_head), NULL, 0);
	msg2main->ops->set_data(msg2main, version, 32, NULL, 0);
	mailbox_post(msg2main);
}


/*位置报告发送*/
static struct LocalInfoHead reset_head =
{
		.type = 0xAACC0201,
		.length = 4
};

void recv_reset_cmd()
{
	struct message *msg2main;
	g_sys_reset_flag = 1;

	cur_count_kt1 = 0;
	clear_array();

	msg2main = msg_factory_produce(g_local_mbx_id);
	msg2main->ops->set_data(msg2main, &reset_head, sizeof(reset_head), NULL, 0);
	mailbox_post(msg2main);

	g_sys_reset_flag = 0;
}

extern G_SYS_CONFIG g_sys_config;

void recv_pos_test_cmd(void *param)
{
	__u16 isr_val;
	g_sys_config.pos_test_item = *((__u32*)param);
	switch(g_sys_config.pos_test_item)
	{
		case 2:
			//自身中断不能关
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val|0x06);
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val|0x06);
			break;

		case 3:
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val&0xFFF9);
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val|0x06);
			break;

		case 4:
			//自身中断不能关
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val|0x06);
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val|0x06);
			break;

		case 5:
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val&0xFFF9);
			isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
			FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val|0x06);
			break;
	}
//	isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
//	printf("FPGA_ISRCLR_REG1 = %x\n",isr_val);
//	isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
//	printf("FPGA_ISRCLR_REG2 = %x\n",isr_val);
}

void recv_sample_result(void *param)
{
	struct Buffer *buf;
	PositionArray* parray;

	SampleResult* pResult = (SampleResult*)param;

	write_array(*pResult);

	if(g_sys_config.pos_test_item == 2)
	{
		buf = buf_factory_produce(g_c1_kt1_buf);
		parray = (PositionArray* )(buf->memory);
		parray->buf = buf;
		parray->sample.count = pResult->count;
		parray->sample.wheel_code = pResult->wheel_code;
		pthread_mutex_lock(&mutex_c1_kt1_list);
		list_add_tail(&(parray->node), &g_c1_kt1_list);
		parray->node.owner = parray;
		pthread_mutex_unlock(&mutex_c1_kt1_list);
	}

	else if(g_sys_config.pos_test_item == 4)
	{
		buf = buf_factory_produce(g_c1_kt2_buf);
		parray = (PositionArray* )(buf->memory);
		parray->buf = buf;
		parray->sample.count = pResult->count;
		parray->sample.wheel_code = pResult->wheel_code;
		pthread_mutex_lock(&mutex_c1_kt2_list);
		list_add_tail(&(parray->node), &g_c1_kt2_list);
		parray->node.owner = parray;
		pthread_mutex_unlock(&mutex_c1_kt2_list);
	}

}


void local_cmd_analyze(SRV_CMD_STR* SrvCmd)
{
	if(SrvCmd->cmdword == 0xAACC0100)
	{
		recv_handshake_cmd();
	}
	else if(SrvCmd->cmdword == 0xAACC0101)
	{
		recv_reset_cmd();
	}
	else if(SrvCmd->cmdword == 0xAACC0102)
	{
		recv_sample_result(SrvCmd->param);
	}
	else if(SrvCmd->cmdword == 0xAACC0103)
	{
		recv_pos_test_cmd(SrvCmd->param);
	}

	else
		;
}
