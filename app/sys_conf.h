/*
 * sys_conf.h
 *
 *  Created on: 2015年8月17日
 *      Author: work
 */

#ifndef SYS_CONF_H_
#define SYS_CONF_H_


#include <linux/types.h>

typedef struct G_SYS_CONFIG
{
	__u32 wheel_code_src;
	__u32 shoot_mode;
	__u32 kick_mode;
	__u32 C1_KT1_delay;
	__u32 C1_C2_delay;
	__u32 KT1_KP1_delay;
	__u32 KP1_KT2_delay;
	__u32 sample_length;
	__u32 video1_valid;
	__u32 video2_valid;
	__u32 pos_test_item;
}G_SYS_CONFIG;


#ifndef SYS_CONF_C_
extern G_SYS_CONFIG g_sys_config;
#endif


void sc_conf_load();
void sc_system_init();

#endif /* SYS_CONF_H_ */
