/*
 * local_socket.c
 *
 *  Created on: 2015年8月12日
 *      Author: work
 */

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <semaphore.h>


#include "thread.h"
#include "debug.h"
#include "buf_factory.h"
#include "msg_factory.h"
#include "mailbox.h"
#include "local_socket.h"
#include "local_protocol.h"

#define UNIX_DOMAIN "/tmp/UNIX.domain"

static __s32 connect_fd;
static __s32 connect_status = 0;
__u32 g_local_mbx_id = 0;
static sem_t gsem_link;

struct sockaddr_un srv_addr;

static SRV_CMD_STR  SrvCmd_str;


void local_connect_tsk(void)
{
	__s32 ret;

	while (1)
	{
		//sem_wait(&gsem_connect);
		usleep(1000);
		if(connect_status)continue;

		connect_fd = socket(PF_UNIX,SOCK_STREAM,0);
		if(connect_fd < 0)
		{
			MSG("creat local socket");
		}

		srv_addr.sun_family = AF_UNIX;
		strcpy(srv_addr.sun_path,UNIX_DOMAIN);

		//connect server
		while(1)
		{
			ret = connect(connect_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
			if(ret != 0)
			{
				usleep(1000);
			}
			else
			{
				MSG("connect to local server path (%s).\n", UNIX_DOMAIN);
				connect_status = 1;
				sem_post(&gsem_link);
				break;
			}
		}
	}
}


void local_send_tsk(void)
{
	struct message msg;
	void *data = NULL;
	__u32 data_num = 0;
	__u32 data_len = 0;
	__u32 i = 0;
	__u32 nhas_send = 0;
	__s32 nsend;

	msg_factory_cast(&msg, g_local_mbx_id);
	while(1)
	{
		mailbox_pend(&msg);
		if (connect_status == 0)
		{
			msg_factory_recycle(&msg);
			continue;
		}
		data_num = msg.ops->get_data_num(&msg);

		for (i = 0; i < data_num; i++)
		{
			data = msg.ops->get_data(&msg, &data_len);
			nhas_send = 0;
			while (nhas_send < data_len)
			{
				nsend = write(connect_fd, data, data_len - nhas_send);
				if (nsend <= 0)
				{
					goto recycle;
				}
				nhas_send = nhas_send + nsend;
			}
		}
recycle:
		msg_factory_recycle(&msg);
	}
}


void local_recv_tsk(void)
{
	__s32 nrecv;
	__u32 nhas_recv;
	struct Buffer *buf;

revReconnect:
	sem_wait(&gsem_link);
	while(1)
	{
		//接受长度与命令
		nhas_recv = 0;
		while (nhas_recv < 8)
		{
			nrecv = read(connect_fd, &SrvCmd_str+nhas_recv, 8-nhas_recv);
			if(nrecv <= 0)
			{
				shutdown(connect_fd, SHUT_RDWR);
				close(connect_fd);
				connect_status = 0;
				goto revReconnect;
			}
			nhas_recv += nrecv;
		}

		SrvCmd_str.length -= 4;
		nhas_recv = 0;
		while (nhas_recv < SrvCmd_str.length)
		{
			nrecv = read(connect_fd, SrvCmd_str.param + nhas_recv, SrvCmd_str.length-nhas_recv);
			if(nrecv <= 0)
			{
				shutdown(connect_fd, SHUT_RDWR);
				close(connect_fd);
				connect_status = 0;
				goto revReconnect;
			}
			nhas_recv += nrecv;
		}

		local_cmd_analyze(&SrvCmd_str);

	}
}


void local_socket_init(void)
{
	g_local_mbx_id = mailbox_create("/rejectlocal");
	SrvCmd_str.param = malloc(1024);

	sem_init(&gsem_link, 0, 0);
	add_new_thread(NULL, (void *)&local_connect_tsk, 7, 0, 8*1024);
	add_new_thread(NULL, (void *)&local_send_tsk, 8, 0, 8*1024);
	add_new_thread(NULL, (void *)&local_recv_tsk, 13, 0, 16*1024);
}
