obj-m += mpu6050_dev_driver.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
all: module dt 

module:
        make -C $(KDIR)  M=$(shell pwd) modules

dt: mpu6050-overlay.dts
        dtc -@ -I dts -O dtb -o mpu6050.dtbo mpu6050-overlay.dts

clean:
        make -C $(KDIR)  M=$(shell pwd) clean
        rm -rf mpu6050.dtbo

