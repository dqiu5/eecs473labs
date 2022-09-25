sudo rm /dev/memory
sudo rmmod partD2
sudo insmod partD2.ko
sudo mknod /dev/memory c 60 0
sudo chmod 666 /dev/memory