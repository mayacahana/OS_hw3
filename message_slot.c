1	/*
2	 * message_slot.c
3	 *
4	 *  Created on: Dec 9, 2017
5	 *      Author: Maya Cahana
6	 */
#undef __KERNEL__
#define __KERNEL__ 
#undef MODULE
#define MODULE 
#include "message_slot.h"

#include <linux/kernel.h>  
#include <linux/module.h>   
#include <linux/fs.h>       /* for register_chrdev */
#include <asm/uaccess.h>  
#include <linux/string.h>  
#include <linux/slab.h> 

MODULE_LICENSE("GPL");

// used to prevent concurent access into the same device
static int dev_open_flag = 0;
static struct chardev_info device_info;
// The message the device will give when asked
static char msg[BUF_LEN];
// device major number
static int major_num;

// define struct for slots
typedef struct msg_slot_struct{
    //TODO
    char msg_buffs[4][128];
    int channel_id;
    int minor_device_id;
    struct msg_slot_struct* next;
} MsgSlot

MsgSlot* head;

//**********HELP FUNCTIONS***********
static unsigned long get_minor(struct file *file_desc){
    iminor(file_desc->f_path.dentry->d_inode);
}

static void MsgSlot insertMsgSlot(MsgSlot* new_node){
    MsgSlot* tmp_node = head;
    if (head==0){
        head = new_node;
        return;
    } else {
        while(tmp_node->next != 0){
            tmp_node = tmp_node->next;
        }
        tmp_node->next = new_node;
    }
}

static MsgSlot* getMsgSlot(int device_id){
    MsgSlot* tmp_node = head;
    while (tmp_node != 0){
        if(tmp_node->minor_device_id == device_id){
            return tmp_node;
        }
        tmp_node = tmp_node->next;
    }
    return 0;
}
//********** DEVICE FUNCTIONS ************

static int device_open(struct inode *inode, struct file *file){
    //TODO check if int or insigned long
    unsigned long device_id = get_minor(file);
    MsgSlot *file_slot = getMsgSlot(device_id);
    // if the device dont exist- file slot is null
    if (!file_slot){
        MsgSlot* new_msgslot = (MsgSlot *)kmalloc(sizeof(MsgSlot), GFP_KERNEL);
        new_msgslot->minor_device_id = device_id;
        new_msgslot->channel_id = -1;
        new_msgslot->next = NULL;
        //HOW TO IMPLEMENT THE BUFFS

        insertMsgSlot(new_msgslot);
    }
    return SUCCESS;
}
// ----------------------
static int device_ioctl(struct inode* inode, unsinged int ioctl_command_id, unsigned int ioctl_param){
    if (MSG_SLOT_CHANNEL == ioctl_command_id){
        int device_id = get_minor(file);
        // get the relevant slot
        MsgSlot *file_slot = getMsgSlot(device_id);
        if (!file_slot){ //not existing
            //check
        } else {
            file_slot->channel_id = (int)ioctl_param;
            // check if we have channel_id

        }
    }
}
// ----------------------
static int device_read(struct file* file, char __user* buffer, size_t length,loff_t* offset){
    int device_id = get_minor(file);
    MsgSlot* file_slot = getMsgSlot(device_id);
    if(!file_slot) {
        //TODO: Error device not 
        return -EINVAL;
    }
    channel_id = file_slot->channel_id;
    // validate
    // copy value from kernel space to user space
    
}