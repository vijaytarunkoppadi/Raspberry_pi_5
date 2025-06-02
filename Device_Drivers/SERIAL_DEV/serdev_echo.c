#include<linux/module.h>
#include<linux/init.h>
#include<linux/serdev.h>
#include<linux/mod_devicetable.h>
#include<linux/property.h>
#include<linux/platform_device.h>
#include<linux/of_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vijay <vijaytarunkoppadi125@gmail.com>");
MODULE_DESCRIPTION("A simple loopback driver for UART Port");

static int serdev_echo_probe(struct serdev_device *serdev);
static void serdev_echo_remove(struct serdev_device *serdev);

static struct of_device_id serdev_echo_ids[]={
        {
                .compatible = "brightlight,echodev",
        },{}
};

MODULE_DEVICE_TABLE(of, serdev_echo_ids);

static struct serdev_device_driver serdev_echo_driver ={
        .probe = serdev_echo_probe,
        .remove = serdev_echo_remove,
        .driver = {
                .name = "serdev-echo",
                .of_match_table = serdev_echo_ids,
        },
};

static size_t serdev_echo_recv(struct serdev_device *serdev,const unsigned char *buffer,size_t size){
        pr_info("serdev_echo - Received %ld bytes with %s\n",size,buffer);
        return serdev_device_write_buf(serdev,buffer,size);
}
static const struct serdev_device_ops serdev_echo_ops ={
        .receive_buf = serdev_echo_recv,
};


static int serdev_echo_probe(struct serdev_device *serdev){
        int status;
        pr_info("serdev_echo probe function\n");
        serdev_device_set_client_ops(serdev,&serdev_echo_ops);
        status = serdev_device_open(serdev);
        if(status){
                pr_info("serdev_echo error opening serial portt\n");
                return -status;
        }

        serdev_device_set_baudrate(serdev,115200);
        serdev_device_set_flow_control(serdev,false);
        serdev_device_set_parity(serdev,SERDEV_PARITY_NONE);

        status = serdev_device_write_buf(serdev,"Type something",sizeof("Type something"));
        pr_info("serdev_echo wrote %d bytes\n",status);
        return 0;
}


static void serdev_echo_remove(struct serdev_device *serdev){
        pr_info("serdev_echo remove function\n");
        serdev_device_close(serdev);
}

static int __init serdev_init(void){
        pr_info("serdev echo loading the driver \n");
        if(serdev_device_driver_register(&serdev_echo_driver)){
                pr_info("serdev echo error could not load driver ");
                return -1;
        }
        return 0;
}

static void __exit serdev_exit(void){
        pr_info("serdev_echo unload driver");
        serdev_device_driver_unregister(&serdev_echo_driver);
}

module_init(serdev_init);
module_exit(serdev_exit);
