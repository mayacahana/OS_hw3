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
#include <linux/uaccess.h>  
#include <linux/string.h>  
#include <linux/slab.h> 

MODULE_LICENSE("GPL");

// used to prevent concurent access into the same device
//static int dev_open_flag = 0;
//static struct chardev_info device_info;

typedef struct message{
    char buffer[BUF_LEN];
    int channel_id;
    struct message* next;
    bool flag_msg;
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
static void insertMessage(MessageSlot* msg_slot, Message* new_channel){
    Message* tmp = msg_slot->msg;
    while (tmp->next != 0){
        tmp = tmp->next;
    }
    tmp->next = new_channel;
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
// static Message* getMessage(int channel_id) {
//     Message* tmp_node = head->msg;
//     while (tmp_node != 0) {
//         if (tmp_node->channel_id == channel_id) {
//             return tmp_node;
//         }
//         tmp_node = tmp_node->next;
//     }
// 	return tmp_node;
// }
static Message* getMessage(MessageSlot* msg_slot, int channel_id){
    Message* tmp = msg_slot->msg;
    while (tmp!=0){
        if (tmp->channel_id == channel_id){
            return tmp;
        }
        tmp = tmp->next;
    }
    return 0;
}

static void freeMem(void){
    Message *tmp_msg, *this_msg_delete;
	MessageSlot *slot_to_delete;
    while (head != NULL){
        tmp_msg = head->msg;
        while (tmp_msg != NULL){
            this_msg_delete = tmp_msg;
            tmp_msg = tmp_msg->next;
            kfree(this_msg_delete);
        }
		slot_to_delete = head;
		head = head->next;
        kfree(slot_to_delete);
    }
}
static MessageSlot* createMessageSlot(int minor_number){
    MessageSlot* new_msg_slot = (MessageSlot *)kmalloc(sizeof(MessageSlot),GFP_KERNEL);
    if (!new_msg_slot){
        return 0;
    }
    new_msg_slot->minor_device_id = minor_number;
    new_msg_slot->device_channel_id = -1;
    new_msg_slot->next = 0;
    new_msg_slot->msg = 0;
    return new_msg_slot;
}
static Message* createMessage(MessageSlot* msg_slot, int channel_id){
    Message* new_msg = (Message *)kmalloc(sizeof(Message),GFP_KERNEL);
    if (!new_msg){
        return 0;
    }
    new_msg->channel_id = channel_id;
    memset(new_msg->buffer,'\0',BUF_LEN);
    new_msg->next = 0;
    new_msg->flag_msg = false;
    return new_msg;
}
//********** DEVICE FUNCTIONS ************

static int device_open(struct inode *inode, struct file *file){
    //TODO check if int or insigned long
    int device_id = get_minor(file);
    MessageSlot *file_slot = getMessageSlot(device_id);
    // if the device dont exist- file slot is null
	printk("entering device_open \n");
    if (!file_slot) {
        file_slot = createMessageSlot(device_id);
        insertMessageSlot(file_slot);
    }
    return SUCCESS;
}
// ----------------------
static long device_ioctl(struct file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param){
	Message* msg;
    printk("enter device_ioctl\n");
    if (MSG_SLOT_CHANNEL == ioctl_command_id){
        int device_id = get_minor(file);
        // get the relevant slot
        MessageSlot *file_slot = getMessageSlot(device_id);
        if (!file_slot){ //not existing
            printk("Device slot not found. \n");
            return -EINVAL;
        } else {
            msg = getMessage(file_slot, ioctl_param);
            if (!msg){
                //No channel with this id. creating one
                Message* new_msg = createMessage(file_slot, ioctl_param);
                insertMessage(file_slot, new_msg);
            }
            file->private_data = (void*)ioctl_param;
        }
    } else {
        printk("Invalid command\n");
        return -EINVAL;
    }
    return SUCCESS;
}

// ----------------------

static int device_read(struct file* file, char __user* buffer, size_t length,loff_t* offset){
    int device_id = get_minor(file);
    MessageSlot* file_slot = getMessageSlot(device_id);
	Message*  msg = 0;
	int channel_id, i;
	printk("entering device_read\n");

	if (length > BUF_LEN || length <= 0) {
        // message is too big or length is invalid. 
        return -ENOSPC;
    }
    if(!file_slot) {
        //TODO: Error device not found
        printk("Slot with this ID was not found\n");
        return -EINVAL;
    }
    channel_id = (int) (uintptr_t)file->private_data;
    // validate channel id
    if (channel_id < 0){
        return -EINVAL;
    }
    msg = getMessage(file_slot,channel_id);
    if (!msg){
        // trying to read from channel that nobody wroted to
        if (!msg->flag_msg)
            return -EWOULDBLOCK;
        else
            return -EINVAL;
    }

    // check copy value from kernel space to user space
    //if (put_user(msg->buffer,buffer)!=0){
    //    printk("Error with buffer\n");
    //    return -EINVAL;
    //}
    //check channel id
    if (length < strlen(msg->buffer)){
        return -ENOSPC;
    }
    for (i=0; i<length && i<BUF_LEN; i++){
        put_user(msg->buffer[i], &buffer[i]);
    }
    return i;
}

// ----------------------

static int device_write (struct file *file, const char __user* buffer, size_t length, loff_t* offset){
    unsigned long device_id = get_minor(file);
	MessageSlot *file_slot = getMessageSlot(device_id);
	Message* current_msg = file_slot->msg;
    Message* tmp = NULL;
	// Message* new_msg = NULL;
	int this_channel_id, i;
	char tmp_ch;
	printk("entering device_write\n");
	if (length > BUF_LEN || length <= 0) {
        // message is too big or length is invalid. 
        return -EINVAL;
    }
    if(!file_slot) {
        // Error slot not found
        printk("Slot with this ID was not found\n");
        return -EINVAL;
    }
    this_channel_id = (int) (uintptr_t)file->private_data;
    // new_msg = getMessage(file_slot,this_channel_id);

    while (current_msg != 0 && current_msg->channel_id != this_channel_id){
        tmp = current_msg;
        current_msg = current_msg->next;
    }
    if (!current_msg) {
        // no messages wrote to this channel
        current_msg = createMessage(file_slot, this_channel_id);
        if (tmp != NULL){
            tmp->next = current_msg;
        } else {
            file_slot->msg = current_msg;
        }
        
    }
    if (strlen(current_msg->buffer) > BUF_LEN)
        return -EINVAL;
    
    if (get_user(tmp_ch, buffer) != 0){
        printk("Buffer not valid \n");
        return -EINVAL;
    }
    for (i=0; i < BUF_LEN && i < length; i++){
        get_user(current_msg->buffer[i], &buffer[i]);
    }
    current_msg->flag_msg = true;
    return SUCCESS;
}

static int device_release (struct inode* inode,
                           struct file*  file){
    return 0;
}

//********** DEVICE SETUP ************
struct file_operations Fops = {
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};
// ----------------------
// Init
// ----------------------
static int __init simple_init(void){
    unsigned int rc = -1; //TODO:check this
    head = 0;
    // register driver capabilities. Obtain major num
    rc = register_chrdev(MAJOR_NUMBER,DEVICE_RANGE_NAME, &Fops);
    // validate
    if (rc < 0) {
        printk(KERN_ALERT "%s registraion failed for  %d\n",DEVICE_FILE_NAME, MAJOR_NUMBER);
    	return ERROR;
    }
	printk(KERN_INFO"message_slot: registered major number %d\n",MAJOR_NUMBER);
    return SUCCESS;
}
// ----------------------
// Exit & cleanup
// ----------------------
static void __exit simple_cleanup(void) {
    // clean memory of the msg slot structs
    freeMem();
    // unregister device
    unregister_chrdev(MAJOR_NUMBER, DEVICE_RANGE_NAME);
}
module_init(simple_init);
module_exit(simple_cleanup);