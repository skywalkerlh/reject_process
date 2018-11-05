/*
 * local_protocol.h
 *
 *  Created on: 2015年8月14日
 *      Author: work
 */

#ifndef LOCAL_PROTOCOL_H_
#define LOCAL_PROTOCOL_H_

#include "local_socket.h"

typedef struct LocalInfoHead
{
	__u32 length;
	__u32 type;
}LocalInfoHead;

void local_cmd_analyze(SRV_CMD_STR* SrvCmd);


#endif /* LOCAL_PROTOCOL_H_ */
