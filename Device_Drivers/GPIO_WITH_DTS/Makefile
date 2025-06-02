obj-m += device.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
all: module dt 

module:
        make -C $(KDIR)  M=$(shell pwd) modules

dt: testoverlay.dts
        dtc -@ -I dts -O dtb -o testoverlay.dtbo testoverlay.dts

clean:
        make -C $(KDIR)  M=$(shell pwd) clean
        rm -rf testoverlay.dtbo
