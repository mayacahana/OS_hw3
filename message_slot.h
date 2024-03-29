// 1	/*
// 2	 * message_slot.h
// 3	 *
// 4	 *  Created on: Dec 9, 2017
// 5	 *      Author: Maya Cahana
// 6	 */


#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128 /*Instructions: A message is a string, at most 128 bytes long. */ 
#define MAX_PATH 260
#define DEVICE_FILE_NAME "message_slot"
#define SUCCESS 0
#define ERROR -1


/*The major number for ioctls */
#define MAJOR_NUMBER 244

/*Set the message of the device driver */
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUMBER, 0, unsigned long)

#endif

