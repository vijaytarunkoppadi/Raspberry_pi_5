Overlay should be add over 
$cp *.dtbo /boot/overlay/
$sudo vi /boot/firmware/config.txt

add the below lines at bottom of the txt 

[all]
enable_uart=1
dtoverlay=serdev_overlay

dtparam=uart0=on

dtparam=spi=on
dtoverlay=spioverlay

dtparam=i2c=on
dtoverlay=mpu6050

esc + 
:wq
$sudo reboot 

