#include<linux/module.h>
#include<linux/init.h>                                                                                                                 #include<linux/mod_devicetable.h>NOR | Minicom 2.9 | VT102 | Offline | ttyACM0                                                         #include<linux/property.h>
#include<linux/platform_device.h>
#include<linux/of_device.h>
#include<linux/gpio/consumer.h>
#include<linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vijay <vijaytarunkoppadi125@gmail.com>");
MODULE_DESCRIPTION("A LKM for gpio device driver using dtsoverlay");

static int gpio_probe(struct platform_device *pdev);
static void gpio_remove(struct platform_device *pdev);

static struct of_device_id my_driver_ids[] ={
        {
                .compatible = "brightlight,mydev",
        },
        {}
};

MODULE_DEVICE_TABLE(of,my_driver_ids);

static struct platform_driver my_driver ={
        .probe  = gpio_probe,
        .remove = gpio_remove,
        .driver = {
                .name = "my_device_driver",
                .of_match_table = my_driver_ids,
        },
}; 

static struct gpio_desc *my_led = NULL;

static struct proc_dir_entry *proc_file;

static ssize_t gpio_write(struct file *filp,const char __user *buf,size_t count,loff_t *off){
        char kbuf[10];
        if(copy_from_user(kbuf,buf,count)){
                return -EFAULT;
        }
        switch(kbuf[0]){
                case '0':
                case '1':
                        gpiod_set_value(my_led,kbuf[0]-'0');
                default:
                        break;
        }
        return count;
}

static struct proc_ops fops={
        .proc_write = gpio_write,
};

static int gpio_probe(struct platform_device *pdev){
        struct device *dev = &pdev->dev;
        const char *label;
        int my_value, ret;

        printk("gpio probe function\n");

        if(!device_property_present(dev,"label")){
                pr_err("gpio Device property label not found\n");
                return -1;
        }

        if(!device_property_present(dev,"my_value")){
                pr_err("gpio Device property my_value not found\n");
                return -1;
        }

        if(!device_property_present(dev,"red-led-gpio")){
                pr_err("gpio Device property red-led-gpio not found\n");
                return -1;
        }

        ret = device_property_read_string(dev,"label",&label);
        if(ret){
                pr_err("gpio could not read 'label'\n");
                return -1;
        }
        pr_info("gpio label : %s\n",label);

        ret = device_property_read_u32(dev,"my_value",&my_value);
        if(ret){
                pr_err("gpio could not read 'label'\n");
                return -1;
        }

        pr_info("gpio my_value : %d\n",my_value);

        my_led = gpiod_get(dev,"red-led",GPIOD_OUT_LOW);
        if(IS_ERR(my_led)){
                pr_err("gpio could not setup the gpio\n");
                return -1;
        }

        proc_file = proc_create("my_led",0666,NULL,&fops);
        if(proc_file == NULL){
                pr_err("Error creating /proc/my-led\n");
                gpiod_put(my_led);
                return -ENOMEM;
        }
        return 0;
}

static void gpio_remove(struct platform_device *pdev){
        pr_info("gpio probe remove function\n");
        gpiod_put(my_led);
        proc_remove(proc_file);
}


static int __init my_init(void){
        pr_info("gpio load the driver\n");
        if(platform_driver_register(&my_driver)){
                pr_err("cannot load gpio driver\n");
                return -1;
        }
        return 0;
}

static void __exit my_exit(void){
        pr_info("gpio unload driver\n");
        platform_driver_unregister(&my_driver);
}

module_init(my_init);
module_exit(my_exit);
