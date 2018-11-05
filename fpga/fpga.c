/*
 * fpga.c
 *
 *  Created on: 2015年8月13日
 *      Author: work
 */

#define FPGA_C_

#include <stddef.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include "debug.h"
#include "fpga.h"
#include "thread.h"
#include "isr.h"

__u32 *fpga_base = NULL;
int kick_fd = 0;

void fpga_context_init()
{
	__s32 fd = 0;

	sigset_t bset, oset;
//	__u16 val = 0;

	/* 进程屏蔽SIGIO信号 */
	sigemptyset(&bset);
	sigaddset(&bset, SIGIO);
	if (pthread_sigmask(SIG_BLOCK, &bset, &oset) != 0)
		perror("pthread_sigmask");

	/* 进程屏蔽SIGPIPE信号 */
	kick_fd = open("/dev/kick_dev", O_RDWR);
	if (kick_fd < 0)
		perror("open");

	/* 告诉驱动当前进程的PID */
	fcntl(kick_fd, F_SETOWN, getpid());
	/* 设置驱动的FASYNC属性，支持异步通知 */
	fcntl(kick_fd, F_SETFL, fcntl(kick_fd, F_GETFL) | FASYNC);


	fd = open("/dev/altera_fpga", O_RDWR);
	if (fd < 0)
		ERROR("open");

	fpga_base = mmap(NULL, 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	add_new_thread(NULL, (void *) &kick1_isr_tsk, 20, 0, 8*1024);
	add_new_thread(NULL, (void *) &kick2_isr_tsk, 20, 0, 8*1024);
}

__u64 fpga_get_current_code()
{
	__u64 code = 0;
	__u32 code_low32 = 0;
	__u32 code_high32 = 0;

	code_low32 = FPGA_READ32(fpga_base, FPGA_WHEELCOUNTER1_REG);
	code_high32 = FPGA_READ32(fpga_base, FPGA_WHEELCOUNTER3_REG);

	code = ((__u64)code_high32 << 32) | (__u64)code_low32;

	return code;
}

