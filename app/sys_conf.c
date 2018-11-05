/*
 * sys_conf.c
 *
 *  Created on: 2015年8月13日
 *      Author: work
 */

#define SYS_CONF_C_

#include <linux/types.h>
#include "key_file.h"
#include "debug.h"
#include "sys_conf.h"

G_SYS_CONFIG g_sys_config;

void sc_conf_load()
{
	__s32 err = 0;

	/* 打开系统配置文件 */
	err = key_file_open("system.conf");
	if (err == -1)
		ERROR("key_file_open");
}


void sc_system_init()
{
	g_sys_config.kick_mode					= key_file_get_int("function", "kick_mode",				0);
	g_sys_config.C1_KT1_delay 		= key_file_get_int("station_delay", "C1_KT1_delay",				1000);
	g_sys_config.KT1_KP1_delay 	= key_file_get_int("station_delay", "KT1_KP1_delay",				1000);
	g_sys_config.KP1_KT2_delay 	= key_file_get_int("station_delay", "KP1_KT2_delay",				1000);
	g_sys_config.C1_C2_delay 			= key_file_get_int("shoot_delay",		"C1_C2_delay", 500);
	g_sys_config.sample_length		= key_file_get_int("function", "sample_length",				1000);
	g_sys_config.video1_valid 		= key_file_get_int("video_channel1", "valid",		0);
	g_sys_config.video2_valid 		= key_file_get_int("video_channel2", "valid",		0);

}


