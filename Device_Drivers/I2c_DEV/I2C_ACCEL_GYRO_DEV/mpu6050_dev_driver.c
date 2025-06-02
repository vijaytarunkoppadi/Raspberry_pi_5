#include<linux/module.h>
#include<linux/init.h>
#include<linux/i2c.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>

#define MPU6050_ADDR            0x68
#define MPU6050_REG_PWR_MGMT    0x6B
#define MPU6050_REG_ACCEL_X     0x3B
#define MPU6050_REG_GYRO_X      0x43


static struct i2c_client *mpu6050_client;
static struct class *mpu6050_class;
static struct cdev mpu6050_cdev;
static dev_t dev_number;

static int mpu6050_read_raw_data(struct i2c_client *client, u8 reg){
        s16 value;
        u8 data[2];
        struct i2c_msg msg[2] = {
                        {.addr = client->addr, .flags = 0 , .len = 1, .buf = &reg},
                        {.addr = client->addr, .flags = I2C_M_RD, .len =2, .buf=data}
        };
        if(i2c_transfer(client->adapter,msg,2) < 0)
                return -EIO;
        value = (data[0]<<8)|data[1];
        return value;
}

static ssize_t mpu6050_read(struct file *file,char __user *buf,size_t len,loff_t *offset){
        s16 accel_x, accel_y, accel_z;
        s16 gyro_x, gyro_y, gyro_z;
        char data[128];
        int size;

        accel_x = mpu6050_read_raw_data(mpu6050_client, MPU6050_REG_ACCEL_X);
        accel_y = mpu6050_read_raw_data(mpu6050_client, MPU6050_REG_ACCEL_X+2);
        accel_z = mpu6050_read_raw_data(mpu6050_client, MPU6050_REG_ACCEL_X+4);

        gyro_x = mpu6050_read_raw_data(mpu6050_client, MPU6050_REG_GYRO_X);
        gyro_y = mpu6050_read_raw_data(mpu6050_client, MPU6050_REG_GYRO_X+2);
        gyro_z = mpu6050_read_raw_data(mpu6050_client, MPU6050_REG_GYRO_X+4);

        size = snprintf(data,sizeof(data),"Accel X:%d Y:%d Z:%d | Gyro X:%d Y:%d Z:%d \n",accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z);

        if(copy_to_user(buf,data,size))
                return -EFAULT;
        return size;
}

static struct file_operations mpu6050_fops={
        .owner  = THIS_MODULE,
        .read   = mpu6050_read
};

static int mpu6050_probe(struct i2c_client *client){
        int ret;
        u8 pwr_mgmt_data[2] = {MPU6050_REG_PWR_MGMT, 0x00};
        mpu6050_client =client;
        ret = i2c_smbus_write_byte_data(client, 0x6B, 0x00);;
        if(ret){
                dev_err(&client->dev, "Failed to wake up MPU6050\n");
                return ret;
        }

        ret = alloc_chrdev_region(&dev_number,0,1,"mpu6050");
        if(ret < 0)
                return ret;
        cdev_init(&mpu6050_cdev, &mpu6050_fops);
        ret = cdev_add(&mpu6050_cdev,dev_number,1);
        if(ret < 0)
                return ret;
        mpu6050_class = class_create("mpu6050_class");
        if(IS_ERR(mpu6050_class))
                return PTR_ERR(mpu6050_class);
        device_create(mpu6050_class,NULL,dev_number,NULL,"mpu6050");
        dev_info(&client->dev, "MPU6050 driver loaded\n");
        return 0;
}

static void mpu6050_remove(struct i2c_client *client){
        device_destroy(mpu6050_class,dev_number);
        class_destroy(mpu6050_class);
        cdev_del(&mpu6050_cdev);
        unregister_chrdev_region(dev_number,1);
        dev_info(&client->dev,"MPU6050 driver removed\n");
}

static const struct i2c_device_id mpu6050_id[]={
        {"mpu6050",0},
        {}
};

MODULE_DEVICE_TABLE(i2c, mpu6050_id);
static struct i2c_driver mpu6050_driver = {
        .driver = {
                .name = "mpu6050_driver",
        },
        .probe  = mpu6050_probe,
        .remove = mpu6050_remove,
        .id_table = mpu6050_id,
};

module_i2c_driver(mpu6050_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vijay <vijaytarunkoppadi125@gmail.com>");
MODULE_DESCRIPTION("Basic I2C driver for MPU6050");
