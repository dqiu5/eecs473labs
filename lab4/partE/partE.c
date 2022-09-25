/**
 * @file    h-bridge device driver
 * @author  Kevin Yang
 * @date    3/20/2017
 * @brief   Solution code for a kernel module that controls a h-bridge. 
 *          Pin mounts and commands are defined below.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/gpio.h>

#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/ioport.h>
//#include <asm/system.h>   /* cli(), *_flags */
#include <linux/uaccess.h>	/* copy_from/to_user */
#include <asm/io.h>

MODULE_LICENSE ("Dual BSD/GPL");


/*
 * Input commands and pin mounts
 */

#define FORWARD 'F'
#define LEFT    'L'
#define BACK    'B'
#define RIGHT   'R'
#define STOP    'S'

//Adjust to add intermediate speeds
#define IDLE    0
#define SPEED   1

//Adjust to reverse motor polarity
int LEFT_MOTOR  = true;
int RIGHT_MOTOR = false;

//These pins are for the RPI3 B, adjust if using a different board
#define EN1 20  //pwm pin, left motor
#define EN2 21  //pwm pin, right motor
#define A_1 6   //Y1, left motor positive
#define A_2 13  //Y2, left motor negative
#define A_3 19  //Y3, right motor positive
#define A_4 26  //Y4, right motor negative


int memory_open (struct inode *inode, struct file *filp);
int memory_release (struct inode *inode, struct file *filp);
ssize_t memory_read (struct file *filp, char *buf, size_t count,
		     loff_t * f_pos);
ssize_t memory_write (struct file *filp, const char *buf, size_t count,
		      loff_t * f_pos);
void memory_exit (void);
int memory_init (void);
//int memory_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

void setPin(int PIN);
void removePin(int PIN);
void moveRobot(char command);
void motorControl(bool ifLeftMotor, char command);

// This file contains snippets of code that can be added to partC.c //

long memory_ioctl (struct file *filp, unsigned int cmd, \
		  unsigned long arg);

struct file_operations memory_fops = 
{
    .read = memory_read,
    .write = memory_write,
    .open = memory_open,
    .release = memory_release,
    .unlocked_ioctl = memory_ioctl
};


module_init (memory_init);
module_exit (memory_exit);

int memory_major = 60;
char *memory_buffer;
int altered = 0;

long 
memory_ioctl (struct file *filp, unsigned int cmd,
 	      unsigned long arg)
{
    printk("<1>in ioctl\n");
    if(cmd==0) 
        altered = 0;	
    else
        altered = 1;
    return(0); // success!
}

int memory_init (void) {
	int result;
	result = register_chrdev (memory_major, "memory", &memory_fops);
	if (result < 0) {
		printk ("Memory: cannot obtain major number %d\n", memory_major);
		return result;
	}

	/* Allocating memory for the buffer */
	memory_buffer = kmalloc (1, GFP_KERNEL);
	if (!memory_buffer) {
		result = -ENOMEM;
		goto fail;
	}

	memset (memory_buffer, 0, 1);
	printk ("Inserting memory module\n");
   
    setPin(EN1);
    setPin(EN2);
    setPin(A_1);
    setPin(A_2);
    setPin(A_3);
    setPin(A_4); 
    
    return 0;

fail:
	memory_exit ();
	return result;
}

void memory_exit (void) {
    unregister_chrdev (memory_major, "memory");
	if (memory_buffer)	{
	    kfree (memory_buffer);
	}
	printk ("Removing memory module\n");
     
    removePin(EN1);
    removePin(EN2);
    removePin(A_1);
    removePin(A_2);
    removePin(A_3);
    removePin(A_4);
    
    printk("GPIO freed, goodbye\n");
}

int memory_open (struct inode *inode, struct file *filp) {
 	return 0;
}

int memory_release (struct inode *inode, struct file *filp) {
 	return 0;
}

ssize_t memory_read (struct file * filp, char *buf, size_t count, loff_t * f_pos) {
  /* Transfering data to user space */
  /* Changing reading position as best suits */
	if (*f_pos == 0) {
		if(copy_to_user (buf, memory_buffer, 1)) return count; //error
		*f_pos += 1;
		return 1;
	} else {
		return 0;
	}
}

ssize_t memory_write (struct file * filp, const char *buf, size_t count, loff_t * f_pos) { 
	int tmp=copy_from_user(memory_buffer, buf, 1);
	if(tmp!=0)
	{
		printk("mem_write error");
		return(count);//just do nothing but say you did all the chars
	}
	f_pos += 1;	

	if (memory_buffer[0] == 'F') {
	    moveRobot(FORWARD);
    }
	if (memory_buffer[0] == 'L') {
	    moveRobot(LEFT);
    }
	if (memory_buffer[0] == 'B') {
	    moveRobot(BACK);
    }
	if (memory_buffer[0] == 'R') {
	    moveRobot(RIGHT);
    }    
	if (memory_buffer[0] == 'S') {
	    moveRobot(STOP);
    }

	return 1;
}

void setPin(int PIN) {
    
    if (!gpio_is_valid(PIN)) {
        printk("Invalid GPIO pin\n");
        return;
    }
    // Your stuff here.
    char label[64];
    sprintf(label, "gpio%d", PIN);
    gpio_request(PIN, label);
    gpio_direction_output(PIN, 0);
    gpio_export(PIN, 1);
    printk("GPIO pin %d exported... Pin state is currently: %d\n", PIN, gpio_get_value(PIN));

}

void removePin(int PIN) {     
	// Your stuff here.
    gpio_set_value(PIN, 0);
    gpio_unexport(PIN);
    gpio_free(PIN);
}

 
void moveRobot(char command) {
    switch(command) {
        case FORWARD:
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, FORWARD);
            break;
        case LEFT:
            if (altered) {
                motorControl(LEFT_MOTOR, FORWARD);
                motorControl(RIGHT_MOTOR, STOP);
            }
            else {
                motorControl(LEFT_MOTOR, STOP);
                motorControl(RIGHT_MOTOR, FORWARD);
            }
            break; 
        case BACK:
            motorControl(LEFT_MOTOR, BACK);
            motorControl(RIGHT_MOTOR, BACK);
            break; 
        case RIGHT:
            if (altered) {
                motorControl(LEFT_MOTOR, STOP);
                motorControl(RIGHT_MOTOR, FORWARD);
            }
            else {
                motorControl(LEFT_MOTOR, FORWARD);
                motorControl(RIGHT_MOTOR, STOP);
            }
            break;
        case STOP:
            motorControl(LEFT_MOTOR, STOP);
            motorControl(RIGHT_MOTOR, STOP);
            break;
        default:
            printk("Illegal command input\n");
            break;
    }   
}

void motorControl(bool ifLeftMotor, char command) {
    int enable      = ifLeftMotor ? EN1 : EN2;
    int motorPos    = ifLeftMotor ? A_1 : A_3;
    int motorNeg    = ifLeftMotor ? A_2 : A_4;

    switch (command) {
        case FORWARD:
            gpio_set_value(enable, SPEED);
            gpio_set_value(motorPos, 1);
            gpio_set_value(motorNeg, 0);
            break;
        case BACK:
            gpio_set_value(enable, SPEED);
            gpio_set_value(motorPos, 0);
            gpio_set_value(motorNeg, 1);
            break;    
        case STOP:
            gpio_set_value(motorPos, 0);
            gpio_set_value(motorNeg, 0);
            break;
        default:
            break;           
    }
}