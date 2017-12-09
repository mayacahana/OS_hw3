#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h"


int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Error: number of command line arguments is invalid\n");
        return -1;
    }
    int file_desc, ret_val;
    int channel_id = atoi(argv[2]);
    char file_name[MAX_PATH] = "/dev/";
    strcat(file_name, argv[2]);
    file_desc = open(file_name, O_RDONLY);
    if (file_desc < 0) {
        printf("Can not open device file: %d\n", file_name);
        return -1;
    }
    ret_val = ioctl(file_desc,IOCTL_SET_ENC, channel_id);
    if (ret_val < 0){
        printf("ioctl set message have failed%d\n", ret_val);
        close(file_desc);
        return -1;
    }
    char buf[BUF_LEN+1];
    int read_num = read(file_dec, BUF_LEN);
    buf[read_num] = '\0';
    close(file_desc);
    printf("The message is: %s\n", buf);
    printf("Status: num of chars read from the device: %d\n", read_num);
    return SUCCESS;
}
    
