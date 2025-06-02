#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/gpio/consumer.h>

static struct gpio_desc *led;

#define IO_LED 571

#define IO_OFFSET 21

dev_t dev =0;
static struct class *gpio_class;
static struct cdev gpio_cdev;

static int __init gpio_init(void);
static void __exit gpio_exit(void);

static int gpio_open(struct inode *inode,struct file *file);
static int gpio_release(struct inode *inode,struct file *file);
static ssize_t gpio_read(struct file *filp,char __user *buff,size_t len,loff_t *off);
static ssize_t gpio_write(struct file *filp,const char __user *buff,size_t len,loff_t *off);

static struct file_operations fops={
        .owner   = THIS_MODULE,
        .open    = gpio_open,
        .release = gpio_release,
        .read    = gpio_read,
        .write   = gpio_write,
};

static int gpio_open(struct inode *inode,struct file *file){
        pr_info("device file opened .. \n");
        return 0;
}
static int gpio_release(struct inode *inode,struct file *file){
        pr_info("device file released .. \n");
        return 0;
}

static ssize_t gpio_read(struct file *filp,char __user *buff,size_t len,loff_t *off){
        uint8_t gpio_state = 0;
        gpio_state = gpiod_get_value(led);
        len = 1;
        if(copy_to_user(buff,&gpio_state,len) > 0){
                pr_err("ERROR: gpio status reading failed\n");
                return -1;
        }
        pr_info(" GPIO pin Status: %d \n",gpio_state);
        return len;
}

static ssize_t gpio_write(struct file *filp,const char __user *buff,size_t len,loff_t *off){
        uint8_t rec_buf[10]={0};
        if(copy_from_user(rec_buf,buff,len) > 0){
                pr_err("Error: gpio write operation failed\n ");
                return -1;
        }
        pr_info("write function : gpio set =%d\n",rec_buf[0]);
        switch(rec_buf[0]){
                case '0':
                        gpiod_set_value(led,0);
                        break;
                case '1':
                        gpiod_set_value(led,1);
                        break;
                default:
                        pr_info("entered data should be 1 or 0 \n");
                        break;
        }
        return len;
}


static int __init gpio_init(void)
{
        if((alloc_chrdev_region(&dev,0,1,"gpio_Dev")) < 0){
                pr_err("cannot allocate major number\n");
                goto r_unreg;
        }
        pr_info("Major = %d Minor =%d\n",MAJOR(dev),MINOR(dev));

        cdev_init(&gpio_cdev,&fops);

        if((cdev_add(&gpio_cdev,dev,1)) < 0){
                pr_err("cannot add the device to the system\n");
                goto r_del;
        }

        if(IS_ERR(gpio_class = class_create("gpio_class"))){
                pr_err("cannot create the struct class \n");
                goto r_class;
        }

        if(IS_ERR(device_create(gpio_class,NULL,dev,NULL,"gpio_dev"))){
                pr_err("cannot create the device \n");
                goto r_device;
        }

        int status;

        led = gpio_to_desc(IO_LED + IO_OFFSET);
        if (!led) {
                printk("gpioctrl - Error getting pin 21\n");
                return -ENODEV;
        }

        status = gpiod_direction_output(led, 0);
        if (status) {
                printk("gpioctrl - Error setting pin 20 to output\n");
                return status;
        }

        gpiod_set_value(led, 1);
        //gpiod_set_value(led, 0);

        return 0;
r_device:
        device_destroy(gpio_class,dev);
r_class:
        class_destroy(gpio_class);
r_del:
        cdev_del(&gpio_cdev);
r_unreg:
        unregister_chrdev_region(dev,1);

        return -1;
}

static void __exit gpio_exit(void)
{
        gpiod_set_value(led, 0);
        device_destroy(gpio_class,dev);
        class_destroy(gpio_class);
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev,1);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vijay <vijaytarunkoppadi125@gmail.com>");
MODULE_DESCRIPTION("An example for using GPIOs without the device tree");
