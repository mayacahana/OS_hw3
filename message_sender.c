// 1	/*
// 2	 * message_sender.c
// 3	 *
// 4	 *  Created on: Dec 9, 2017
// 5	 *      Author: Maya Cahana
// 6	 */

#include "message_slot.h"

#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[]) 

{
    if (argc < 3){
        printf("Error: num of command line args is invalid \n");
        return ERROR;
    }
    int file_desc, ret_val;
    int channel_id = atoi(argv[2]);
    //char file_name[MAX_PATH] = "/dev/";
    //strcat(file_name, argv[1]);
    file_desc = open(argv[1], O_RDWR);
    if (file_desc < 0){
        printf("Can not open device file: %s\n");
        return ERROR;
    } 
    ret_val = ioctl(file_desc, MSG_SLOT_CHANNEL, channel_id);
    if (ret_val < 0){
        printf("Ioctl set meddage failed. %d\n",ret_val);
        close(file_desc);
        return ERROR;
    }
    int num_written = write(file_desc,argv[3], strlen(argv[3]));
    close(file_desc);
    printf("Status: number of chars written to the device is: %d\n", num_written);
    return SUCCESS;
}