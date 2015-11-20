#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>

#define DEVICE_NAME "comp309_char_buttons"
#define KEY1        S5PV210_GPH2(0)
#define KEY2        S5PV210_GPH2(1)
#define KEY3        S5PV210_GPH2(2)
#define KEY4        S5PV210_GPH2(3)
#define KEY5	    S5PV210_GPH3(0)
#define KEY6        S5PV210_GPH3(1)
#define KEY7        S5PV210_GPH3(2)
#define KEY8        S5PV210_GPH3(3)
#define LED_ON      0
#define LED_OFF     1
#define LED1 	    S5PV210_GPJ2(0)
#define LED2	    S5PV210_GPJ2(1)
#define LED3	    S5PV210_GPJ2(2)
#define LED4	    S5PV210_GPJ2(3)
#define N_D         8                      /*Number of Devices*/
#define S_N         1                      /*The start minor number*/

static wait_queue_head_t button_waitq[8];
static int condition = 0;
static unsigned int LED[]= {LED1, LED2, LED3, LED4};
static unsigned int BUTTON[]= {KEY1,KEY2,KEY3,KEY4,KEY5,KEY6,KEY7,KEY8};
static int current_value[] = {0,0,0,0,0,0,0,0};
static int          major;
static int          minor;
static dev_t        devno;
static struct cdev  dev_lab6;

static irqreturn_t zili_demo_char_button_interrupt(int irq, void *dev_id)
{
	int i=0;
	for(i=0;i<8;i++){
		if(gpio_get_value(BUTTON[i])==0){
			current_value[i] = 1;
			printk("[KERNEL] KEY: %d\n", i);
			condition = 1;
			wake_up_interruptible(&button_waitq[i]);
		}
	}
	return IRQ_HANDLED;

}

static int zili_demo_char_button_open(struct inode *inode, struct file *file)
{
	int irq;
	int ret = 0;
	file->private_data = MINOR(inode->i_rdev);
	printk("Device " DEVICE_NAME " open (Minor=%d).\n", (int)file->private_data);
	irq = gpio_to_irq(BUTTON[(int)file->private_data-1]);
	ret = request_irq(irq, zili_demo_char_button_interrupt, IRQ_TYPE_EDGE_BOTH, "KEY", NULL);
	if(ret != 0)
	{
	printk("Fail to Register IRQ for KEY%d\n",minor);
	}
	return 0;
}

static int zili_demo_char_button_release(struct inode *inode, struct file *file)
{
	gpio_set_value (LED1, LED_OFF);
	gpio_set_value (LED2, LED_OFF);
	gpio_set_value (LED3, LED_OFF);
	gpio_set_value (LED4, LED_OFF);
	int irq;
	int i=0;
	irq = gpio_to_irq(BUTTON[(int)file->private_data-1]);
	free_irq(irq, NULL);
	printk("Device " DEVICE_NAME " release.\n");
	return 0;
}



static int zili_demo_char_button_read(struct file *filp, char __user *buff,	size_t count, loff_t *offp)
{
	unsigned long ret;
	condition = 0;
	int i;
	i=filp->private_data-1;
	wait_event_interruptible(button_waitq[i], condition);
	if(current_value[i]==1){
		gpio_set_value(LED[i/2],i%2);
		printk("i=%d, led=%d, write %d\n",i,i/2,i%2);
		current_value[i]=0;
	}
}

static struct file_operations zili_demo_fops = {
	owner   : THIS_MODULE,
	open    : zili_demo_char_button_open,
	read    : zili_demo_char_button_read,
	release : zili_demo_char_button_release,
};



static int __init zili_demo_char_button_init(void)
{
	int ret;
	int i=0;
	for(i=0;i<8;i++)
		init_waitqueue_head(&button_waitq[i]);
	s3c_gpio_cfgpin(LED1, S3C_GPIO_OUTPUT);
	gpio_set_value (LED1, LED_OFF);
	s3c_gpio_cfgpin(LED2, S3C_GPIO_OUTPUT);
	gpio_set_value (LED2, LED_OFF);
	s3c_gpio_cfgpin(LED3, S3C_GPIO_OUTPUT);
	gpio_set_value (LED3, LED_OFF);
	s3c_gpio_cfgpin(LED4, S3C_GPIO_OUTPUT);
	gpio_set_value (LED4, LED_OFF);
	
	/*Register a major number*/
	ret = alloc_chrdev_region(&devno, S_N, N_D, DEVICE_NAME);
	if(ret < 0)
	{
		printk("Device " DEVICE_NAME " cannot get major number.\n");
		return ret;
	}
	major = MAJOR(devno);
	printk("Device " DEVICE_NAME " initialized (Major Number -- %d).\n", major);
	/*Register a char device*/
	cdev_init(&dev_lab6, &zili_demo_fops);
	dev_lab6.owner = THIS_MODULE;
	dev_lab6.ops   = &zili_demo_fops;
	ret = cdev_add(&dev_lab6, devno, N_D);
	if(ret)
	{
		printk("Device " DEVICE_NAME " register fail.\n");
		return ret;
	}
	return 0;
}

static void __exit zili_demo_char_button_exit(void)
{
	cdev_del(&dev_lab6);
	unregister_chrdev_region(devno, N_D);
	printk("Device " DEVICE_NAME " unloaded.\n");
}

module_init(zili_demo_char_button_init);
module_exit(zili_demo_char_button_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dr. Zili Shao <cszlshao@comp.polyu.edu.hk>");
MODULE_DESCRIPTION("Char Driver Development: BUTTON Detection. Course: HK POLYU COMP 309");