#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
main()
{
        
    int a=open("/dev/memory",O_RDWR);
    char inputkey;
    int keypressed = 1;
    while (1) {
        printf("Enter r to toggle polarity.\n");
        inputkey = getchar();
        if(inputkey == 'r' && keypressed){
            ioctl(a,1,0);
            keypressed = 0;
        }
        else if(inputkey == 'r' && !keypressed){
            ioctl(a,0,0);
            keypressed = 1;
        }
        
    }
}