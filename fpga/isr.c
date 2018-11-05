/*
 * isr.c
 *
 *  Created on: 2015年8月12日
 *      Author: work
 */


#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#include "fpga.h"
#include "debug.h"
#include "position.h"
#include "array.h"
#include "sys_conf.h"
#include "message.h"
#include "mailbox.h"
#include "msg_factory.h"

static __u64 kick1_wheel_code = 0;
static __u64 kick2_wheel_code = 0;
static __u64 kt1_wheel_code = 0;
static __u64 kt2_wheel_code = 0;
//static __u64 kp1_wheel_code = 0;
//static __u64 kp2_wheel_code = 0;
//static __u64 kft1_wheel_code = 0;
//static __u64 kft2_wheel_code = 0;

static 	struct timeval kp1_time;
static 	struct timeval kp2_time;
static 	struct timeval kft1_time;
static 	struct timeval kft2_time;

extern int kick_fd;

//static __u32 num1 = 1;
//static __u32 num2 = 1;
//static __u32 num3 = 1;
////
//static __u32 num4 = 1;
//static __u32 num5 = 1;
//static __u32 num6 = 1;

void kick1_isr_tsk(void)
{
	__u16 status = 0;
	__u16 isr_val;
	struct message *msg;
	__u32 kick1_isr_num = 1;
	sigset_t waitset;
	siginfo_t info;
	__s32 rc;



	pthread_t ppid = pthread_self();
	pthread_detach(ppid);

	sigemptyset(&waitset);
	sigaddset(&waitset, SIGIO);

//	printf("Thread:%s start.\n",__FUNCTION__);

	while (1)
	{
		rc = sigwaitinfo(&waitset, &info);
		if (rc != -1)
		{
			if (info.si_signo == SIGIO)
			{
//				printf("kick1 isr %ds.\n",kick1_isr_num++);
				status = FPGA_READ16(fpga_base, FPGA_ISRSTATE_REG1);
				while (status)
				{
					/* 踢废中断位置有样品经过*/
					if (status & 0x01)
					{
						isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
						FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val|0x1);
						FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val&0xFFFE);

//						printf("kick1 position isr %ds.\n",num1++);

						msg = msg_factory_produce(g_pos_kick1_mbx_id);
						kick1_wheel_code = fpga_get_current_code();
//						printf("kick1 position wheel %d.\n",kick1_wheel_code);
						msg->ops->set_data(msg, &kick1_wheel_code, sizeof(kick1_wheel_code), NULL, 0);
						mailbox_post(msg);
						if(g_sys_config.pos_test_item == 2)
						{
							kt1_wheel_code = kick1_wheel_code;
							msg = msg_factory_produce(g_pos_kt1_mbx_id);
							msg->ops->set_data(msg, &kt1_wheel_code, sizeof(kt1_wheel_code), NULL, 0);
							mailbox_post(msg);
						}
					}

					/* 下发踢废信号*/
					if(status & 0x0002)
					{
						isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
						FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val|0x2);
						FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val&0xFFFD);

//						printf("kick1 cmd isr %ds.\n",num2++);

						if(g_sys_config.pos_test_item == 3)
						{
							msg = msg_factory_produce(g_pos_kp1_mbx_id);
//							kp1_wheel_code = fpga_get_current_code();
//							msg->ops->set_data(msg, &kp1_wheel_code, sizeof(kp1_wheel_code), NULL, 0);
//							printf("kp1_wheel_code =  %ds.\n",kp1_wheel_code);
							gettimeofday(&kp1_time, NULL);
							msg->ops->set_data(msg, &kp1_time, sizeof(kp1_time), NULL, 0);
							mailbox_post(msg);
						}
					}

					/* 下发踢废反馈信号*/
					if(status & 0x0004)
					{
						isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG1);
						FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val|0x4);
						FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG1, isr_val&0xFFFB);

//						printf("kick1 feedback isr %ds.\n",num3++);

						if(g_sys_config.pos_test_item == 3)
						{
							msg = msg_factory_produce(g_pos_kft1_mbx_id);
//							kft1_wheel_code = fpga_get_current_code();
////							printf("kft1_wheel_code =  %ds.\n",kft1_wheel_code);
//							msg->ops->set_data(msg, &kft1_wheel_code, sizeof(kft1_wheel_code), NULL, 0);
							gettimeofday(&kft1_time, NULL);
							msg->ops->set_data(msg, &kft1_time, sizeof(kft1_time), NULL, 0);
							mailbox_post(msg);
						}
					}

					status = FPGA_READ16(fpga_base, FPGA_ISRSTATE_REG1);
				}
			}
		}
		else
			MSG("sigwaitinfo");
	}
}




void kick2_isr_tsk(void)
{
	__u16 status = 0;
	__u16 isr_val;
	__u32 kick2_isr_num = 1;
	__s32 err;
	struct message *msg;
	__u32 readbuf;

//	printf("Thread:%s start.\n",__FUNCTION__);

	while (1)
	{
		err = read(kick_fd, &readbuf, sizeof(readbuf));
		if(err != -1)
		{
			//printf("kick2 isr %ds.\n",kick2_isr_num++);
			status = FPGA_READ16(fpga_base, FPGA_ISRSTATE_REG2);
			while (status)
			{
				/* 踢废中断位置有样品经过*/
				if (status & 0x01)
				{
					isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
					FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val|0x1);
					FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val&0xFFFE);

//					printf("kick2 position isr %ds.\n",num4++);

					msg = msg_factory_produce(g_pos_kick2_mbx_id);
					kick2_wheel_code = fpga_get_current_code();
//					printf("kick2 position wheel %d.\n",kick2_wheel_code);
					msg->ops->set_data(msg, &kick2_wheel_code, sizeof(kick2_wheel_code), NULL, 0);
					mailbox_post(msg);
					if(g_sys_config.pos_test_item == 4)
					{
						kt2_wheel_code = kick2_wheel_code;
						msg = msg_factory_produce(g_pos_kt2_mbx_id);
						msg->ops->set_data(msg, &kt2_wheel_code, sizeof(kt2_wheel_code), NULL, 0);
						mailbox_post(msg);
					}
				}

				/* 下发踢废信号*/
				if(status & 0x0002)
				{
					isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
					FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val|0x2);
					FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val&0xFFFD);

//					printf("kick2 cmd isr %ds.\n",num5++);

					if(g_sys_config.pos_test_item == 5)
					{
						msg = msg_factory_produce(g_pos_kp2_mbx_id);
//						kp2_wheel_code = fpga_get_current_code();
//						msg->ops->set_data(msg, &kp2_wheel_code, sizeof(kp2_wheel_code), NULL, 0);
						gettimeofday(&kp2_time, NULL);
						msg->ops->set_data(msg, &kp2_time, sizeof(kp2_time), NULL, 0);
						mailbox_post(msg);
					}
				}

				/* 踢废反馈信号*/
				if(status & 0x0004)
				{
					isr_val = FPGA_READ16(fpga_base, FPGA_ISRCLR_REG2);
					FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val|0x4);
					FPGA_WRITE16(fpga_base, FPGA_ISRCLR_REG2, isr_val&0xFFFB);

//					printf("kick2 feedback isr %ds.\n",num6++);

					if(g_sys_config.pos_test_item == 5)
					{
						msg = msg_factory_produce(g_pos_kft2_mbx_id);
//						kft2_wheel_code = fpga_get_current_code();
//						msg->ops->set_data(msg, &kft2_wheel_code, sizeof(kft2_wheel_code), NULL, 0);
						gettimeofday(&kft2_time, NULL);
						msg->ops->set_data(msg, &kft2_time, sizeof(kft2_time), NULL, 0);
						mailbox_post(msg);
					}
				}

				status = FPGA_READ16(fpga_base, FPGA_ISRSTATE_REG2);
			}
		}
		else
			MSG("sigwaitinfo");
	}
}
