/*
* message_slot.c
*
*  Created on: Dec 9, 2017
*      Author: Maya Cahana
*/
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

typedef struct message{
    char buffer[BUF_LEN];
    int channel_id;
    struct message* next;
} Message;

// define struct for slots
typedef struct msg_slot_struct{
    int minor_device_id;
    int device_channel_id;
    struct msg_slot_struct* next;
    Message* msg;
} MessageSlot;


MessageSlot* head;

//**********HELP FUNCTIONS***********

static int get_minor(struct file *file_desc){
    return iminor(file_desc->f_inode);
}

static void insertMessageSlot(MessageSlot* new_node){
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
    while (tmp_node != 0) {
        if (tmp_node->minor_device_id == device_id) {
            return tmp_node;
        }
        tmp_node = tmp_node->next;
    }
    return 0;
}
static Message* getMessage(int channel_id){
    Message* tmp_node = head->msg;
    while (tmp_node != 0){
        if (tmp_node->channel_id == channel_id) {
            return tmp_node;
        }
        tmp_node = tmp_node->next;
    }
}

static void freeMem(void){
    Message* tmp_msg;
    MessageSlot* tmp_slot;
    while (head != NULL){
        tmp_slot = head;
        head = head->next;
        while (tmp_slot->msg != NULL){
            tmp_msg = tmp_slot->msg;
            tmp_slot->msg = tmp_slpt->msg.next;
            kfree(tmp_msg);
        }
        kfree(tmp->slot.next);
    }
}
//********** DEVICE FUNCTIONS ************

static int device_open(struct inode *inode, struct file *file){
    //TODO check if int or insigned long
    unsigned long device_id = get_minor(file);
    MessageSlot *file_slot = getMessageSlot(device_id);
    // if the device dont exist- file slot is null
    if (!file_slot) {
        MessageSlot* new_MessageSlot = (MessageSlot *)kmalloc(sizeof(MessageSlot), GFP_KERNEL);
        new_MessageSlot->minor_device_id = device_id;
        new_MessageSlot->device_channel_id = -1;
        new_MessageSlot->next = NULL;
        new_MessageSlot->msg = NULL;
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
            printk("Channel index not found. \n");
            return -EINVAL;
        } else {
            file_slot->channel_id = (int)ioctl_param;
        }
    } else {
        printk("Invalid command\n");
        return -EINVAL;
    }
    return SUCCESS;
}

// ----------------------

static int device_read(struct file* file, char __user* buffer, size_t length,loff_t* offset){
    if (length > BUF_LEN || length <= 0) {
        // message is too big or length is invalid. 
        return -ENOSPC;
    }
    int device_id = get_minor(file);
    MessageSlot* file_slot = getMessageSlot(device_id);
    if(!file_slot) {
        //TODO: Error device not found
        printk("Slot with this ID was not found\n");
        return -EINVAL;
    }
    channel_id = file_slot->channel_id;
    // validate
    msg = getMessage(channel_id);
    if (!msg){
        // trying to read from channel that nobody wroted to
        return -EWOULDBLOCK;
    }
    // check copy value from kernel space to user space
    if (put_user(msg->buffer,buffer)!=0){
        printk("Error with buffer\n");
        return -EINVAL;
    }
    //check channel id
    if (channel_id < 0){ //no channel has been set
        printk("Channel was not set\n");
        return -EINVAL;
    }
    int i;
    for (i=0; i<length && i<BUF_LEN; i++){
        put_user(msg->buffer[i], &buffer[i])
    }
    return i;
}

// ----------------------

static int device_write (struct file *file, const char __user* buffer, size_t length, loff_t * offset ){
    if (length > BUF_LEN || length <= 0) {
        // message is too big or length is invalid. 
        return -EINVAL;
    }
    unsigned long device_id = get_minor(file);
    MessageSlot *file_slot = getMessageSlot(device_id);
    if(!file_slot) {
        // Error slot not found
        printk("Slot with this ID was not found\n")
        return -EINVAL;
    }
    this_channelid = file_slot->channel_id;
    Message* current_msg = file_slot->msg;
    Message* tmp = NULL;
    while (current_msg!=NULL && current_msg->channel_id != this_channelid){
        tmp = current_msg;
        current_msg = current_msg->next;
    }
    if (!msg) {
        // no messages wrote to this channel
        new_msg = (Message *)kmalloc(sizeof(Message),GFP_KERNEL)
        if (!new_msg)
            return -ENOMEM;
        new_msg->next = NULL;
        new_msg->channel_id = this_channelid;
        if (tmp != NULL){
            tmp->next = new_msg;
        } else {
            file_slot->msg = current_msg;
        }
    }
    // checking with the buffer
    char tmp;
    if (get_user(tmp, buffer) != 0){
        printk("Buffer not valid \n");
        return -EINVAL;
    }
    int i;
    for (i=0; i < BUF_LEN && i < length; i++){
        get_user(current_msg->buffer[i], &buffer[i]);
    }
    return SUCCESS;
}

static int device_release{
    return 0;
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
    freeMem();
    // unregister device
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}
module_init(simple_init);
module_exit(simple_cleanup);