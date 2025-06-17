obj-m += mcp3008.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
all: module dt 

module:
        make -C $(KDIR)  M=$(shell pwd) modules

dt: mcp3008-overlay.dts
        dtc -@ -I dts -O dtb -o mcp3008.dtbo mcp3008-overlay.dts

clean:
        make -C $(KDIR)  M=$(shell pwd) clean
        rm -rf mcp3008.dtbo

