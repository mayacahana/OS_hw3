1	/*
2	 * message_slot.h
3	 *
4	 *  Created on: Dec 9, 2017
5	 *      Author: Maya Cahana
6	 */

#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128 /*Instructions: A message is a string, at most 128 bytes long. */ 
#define MAX_PATH 260
#define DEVICE_FILE_NAME "message_slot"
#define SUCCESS 0
#define ERROR -1

/*The major number for ioctls */
#define MAJOR_NUM 245


#endif

