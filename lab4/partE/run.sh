sudo rm /dev/memory
sudo rmmod partE
sudo insmod partE.ko
sudo mknod /dev/memory c 60 0
sudo chmod 666 /dev/memory