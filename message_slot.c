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


// define struct for slots
typedef struct msg_slot_struct{
    //TODO
    //char msg_buffs[4][128];
    int minor_device_id;
    int device_channel_id;
    struct msg_slot_struct* next;
    Message* msg;
} MessageSlot

typedef struct message{
    char buffer[BUF_LEN];
    int channel_id;
    struct message* next;
} Message

MessageSlot* head;

//**********HELP FUNCTIONS***********
static unsigned long get_minor(struct file *file_desc){
    iminor(file_desc->f_path.dentry->d_inode);
}

static void MessageSlot insertMessageSlot(MessageSlot* new_node){
    MessageSlot* tmp_node = head;
    if (head == 0){
        head = new_node;
        return;
    } else {
        while(tmp_node->next != 0){
            tmp_node = tmp_node->next;
        }
        tmp_node->next = new_node;
    }
}

static MessageSlot* getMessageSlot(int device_id) {
    MessageSlot* tmp_node = head;
    while (tmp_node != 0){
        if(tmp_node->minor_device_id == device_id) {
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
    MessageSlot *file_slot = getMessageSlot(device_id);
    // if the device dont exist- file slot is null
    if (!file_slot){
        MessageSlot* new_MessageSlot = (MessageSlot *)kmalloc(sizeof(MessageSlot), GFP_KERNEL);
        new_MessageSlot->minor_device_id = device_id;
        new_MessageSlot->channel_id = -1;
        new_MessageSlot->next = NULL;
        //HOW TO IMPLEMENT THE BUFFS

        insertMessageSlot(new_MessageSlot);
    }
    return SUCCESS;
}
// ----------------------
static int device_ioctl(struct inode* inode, unsinged int ioctl_command_id, unsigned int ioctl_param){
    if (MSG_SLOT_CHANNEL == ioctl_command_id){
        int device_id = get_minor(file);
        // get the relevant slot
        MessageSlot *file_slot = getMessageSlot(device_id);
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
    MessageSlot* file_slot = getMessageSlot(device_id);
    if(!file_slot) {
        //TODO: Error device not 
        return -EINVAL;
    }
    channel_id = file_slot->channel_id;
    // validate
    // copy value from kernel space to user space
    int i=0;
    if (put_user(file_slot->msg_buffs[channel_id][0],buffer)!=0){

    }
}

static int device_write{

}

static int device_release{

}

//********** DEVICE SETUP ************
struct file_operations Fops = {
    .read = device_read;
    .write = device_write;
    .unlocked_ioctl = device_ioctl;
    .open = device_open;
    .release = device_release;
};
// ----------------------
// Init
// ----------------------
static int __init simple_init(void){
    unsigned int rc = -1; //TODO:check this
    head = 0;
    // register driver capabilities. Obtain major num
    rc = register_chrdev(MAJOR_NUM,DEVICE_RANGE_NAME, &Fops);
    // validate
    if (rc < 0) {
        printk(KERN_ALERT "%s registraion failed for  %d\n",DEVICE_FILE_NAME, MAJOR_NUM);
    return ERROR;
    }
    return SUCCESS;
}
// ----------------------
// Exit & cleanup
// ----------------------
static void __exit simple_cleanup(void) {
    // clean memory of the msg slot structs
    //TODO
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}
module_init(simple_init);
module_exit(simple_cleanup);