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


