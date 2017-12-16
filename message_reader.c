
/*
 * message_reader.c
 *
 *  Created on: Dec 9, 2017
 *      Author: Maya Cahana
*/
#include "message_slot.h"

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl *//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Error: number of command line arguments is invalid\n");
        return ERROR;
    }
    int file_desc, ret_val;
    int channel_id = atoi(argv[2]);
    // char file_name[MAX_PATH] = "/dev/";
    // strcat(file_name, argv[1]);
    file_desc = open(argv[1], O_RDWR);
    if (file_desc < 0) {
        printf("Can not open device file: %d\n", file_name);
        return ERROR;
    }
    // sets device message slot to the right channel
    ret_val = ioctl(file_desc, MSG_SLOT_CHANNEL, channel_id);
    if (ret_val < 0){
        printf("ioctl set message have failed%d\n", ret_val);
        close(file_desc);
        return ERROR;
    }
    char buf[BUF_LEN+1];
    // reading message from device message slot channel without a loop = reading entire buffer or fail
    int read_num = read(file_desc, buf, BUF_LEN);
    buf[read_num] = '\0';
    close(file_desc);
    printf("The message is: %s\n", buf);
    printf("Status: num of chars read from the device: %d\n", read_num);
    return SUCCESS;
}
    
