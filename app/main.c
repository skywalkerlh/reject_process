/*
 * main.c
 *
 *  Created on: 2015年8月12日
 *      Author: work
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "sys_conf.h"
#include "local_socket.h"
#include "array.h"
#include "position.h"
#include "msg_factory.h"
#include "fpga.h"

void main(void)
{
	__u32 i=0;
	/* 进程屏蔽SIGPIPE信号 */
	signal(SIGPIPE, SIG_IGN);

	/* 加载系统配置文件 */
	sc_conf_load();

	create_msg_factory();

	/*系统初始化*/
	sc_system_init();

	fpga_context_init();

	local_socket_init();

	array_buf_init();

	position_context_init();

	while(1)
	{
		sleep(1);
	}

}
