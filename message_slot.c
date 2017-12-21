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


MessageSlot* head = NULL;

//**********HELP FUNCTIONS***********

static int get_minor(struct file *file_desc){
    return iminor(file_desc->f_inode);
}

static void insertMessageSlot(MessageSlot* new_node){
    MessageSlot* tmp_node = head;
    if (head == 0){
    	//printk("head init\n");
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
    if (tmp == 0) {
    	msg_slot->msg = new_channel;
    	return;
    }
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
    printk("return 0");

    return 0;
}

static Message* getMessage(MessageSlot* msg_slot, int channel_id){
    Message* tmp = msg_slot->msg;
    while (tmp != 0){
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
    memset(new_msg->buffer,0,BUF_LEN);
    new_msg->next = 0;
    new_msg->flag_msg = false;
    return new_msg;
}
//********** DEVICE FUNCTIONS ************

static int device_open(struct inode *inode, struct file *file){
    //TODO check if int or insigned long
    int device_id = get_minor(file);
    MessageSlot *file_slot;
    printk("device id: %d\n", device_id);
    file_slot = getMessageSlot(device_id);
    // if the device dont exist- file slot is null
    if (!file_slot) {
    	printk("create new message slot\n");
        file_slot = createMessageSlot(device_id);
        if (!file_slot){
        	return -ENOMEM;
        }
        insertMessageSlot(file_slot);
    }
    //printk("done device_open \n");
    return SUCCESS;
}
// ----------------------
static long device_ioctl(struct file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param){
	Message* msg;
	MessageSlot *file_slot;
    // printk("enter device_ioctl\n");
    if (MSG_SLOT_CHANNEL == ioctl_command_id){
        int device_id = get_minor(file);
        // get the relevant slot
        file_slot = getMessageSlot(device_id);
        if (!file_slot){ //not existing
            printk("Device slot not found. \n");
            return -EINVAL;
        } else {
            msg = getMessage(file_slot, ioctl_param);
            if (!msg){
                //No channel with this id. creating one
                Message* new_msg = createMessage(file_slot, ioctl_param);
                if (!new_msg) {
                	return -ENOMEM;
                }
                insertMessage(file_slot, new_msg);
            }
            file->private_data = (void*)ioctl_param;
        }
    file->private_data = (void*)ioctl_param;
    } else {
        printk("Invalid command\n");
        return -EINVAL;
    }
    // printk("DONE ioctl\n");
    return SUCCESS;
}

// ----------------------

static int device_read(struct file* file, char __user* buffer, size_t length,loff_t* offset){
    int device_id = get_minor(file);
    MessageSlot* file_slot = getMessageSlot(device_id);
	Message*  msg = 0;
	int channel_id, i;
    if(!file_slot) {
        // Error device not found
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
        printk("Channel with this ID was not found\n");
        return -EINVAL;
    } else {
    	//if (msg->buffer[0] == '\0')
    	if (strlen(msg->buffer) == 0){
            return -EWOULDBLOCK;
        }
    }
	if (length > BUF_LEN) {
        // message is too big or length is invalid. 
        return -ENOSPC;
    }
    if (length < strlen(msg->buffer)){
        return -ENOSPC;
    }
    for (i=0; i < strlen(msg->buffer)+1 && i<BUF_LEN; i++) {
    	if (put_user(msg->buffer[i], &buffer[i]) != 0) {
    		return -EFAULT;
        }
    }
    printk("num of read: %d\n", i);
    return i;
}

// ----------------------

static int device_write (struct file *file, const char __user* buffer, size_t length, loff_t* offset){
    unsigned long device_id = get_minor(file);
	MessageSlot *file_slot = getMessageSlot(device_id);
	Message* current_msg;
	int this_channel_id, i, cnt=0;
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
	// printk("this channel id: %d\n", this_channel_id);
	current_msg = getMessage(file_slot, this_channel_id);
    if (!current_msg) {
    	return -EINVAL;
    }

    if (length > BUF_LEN) {
        return -EINVAL;
    }
	printk("the length is: %d\n", length);
    for (i=0; i < BUF_LEN && i < length; i++){
        if (get_user(current_msg->buffer[i], &buffer[i]) != 0){
        	return -EFAULT;
        	}
        cnt++;
    }
    current_msg->buffer[length] = '\0';
    current_msg->flag_msg = true;
    // printk("DONE writing to buff\n");
    printk("num of write: %d\n", cnt);
    return cnt;
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
    printk("simple init");

    head = 0;
    // register driver capabilities. Obtain major num
    rc = register_chrdev(MAJOR_NUMBER,DEVICE_RANGE_NAME, &Fops);
    printk("Simple init after register chrdev");
	printk("rc: %d",rc);
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