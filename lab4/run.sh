sudo rm /dev/memory
sudo rmmod partC
sudo insmod partC.ko
sudo mknod /dev/memory c 60 0
sudo chmod 666 /dev/memory