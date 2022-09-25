#include <stdio.h>

#define STRINGSIZE 64

#define SYSDIREXP "/sys/class/gpio/export"
#define SYSDIRVAL "/sys/class/gpio/gpio%d/value"
#define SYSDIRDIR "/sys/class/gpio/gpio%d/direction"

void pinMode(int pin, int mode){
    char dir[STRINGSIZE];
    sprintf(dir, SYSDIRDIR, pin);
    printf(dir);

    FILE *fp;
    fp = fopen(dir, "w+");

    switch(mode) {
        case 0:
            fprintf(fp, "out");
            break;
        case 1:
            fprintf(fp, "in");
            break;
        default:
            break;
    }
    fclose(fp);
}

void digitalWrite(int pin, int val) {
    char dir[STRINGSIZE];
    sprintf(dir, SYSDIRVAL, pin);

    char val_c[STRINGSIZE];
    sprintf(val_c, "%d", val);

    FILE *fp;
    fp = fopen(dir, "w+");
    fprintf(fp, val_c);
    fclose(fp);
}

void exportPin(int pin) {
    char pin_c[STRINGSIZE];
    sprintf(pin_c, "%d", pin);
    //printf(pin_c);

    FILE *fp;
    fp = fopen(SYSDIREXP, "w+");
    fprintf(fp, "23");
    fclose(fp);
}

int main() {
    //exportPin(23);
    pinMode(23, 0);
    int val = 0;
    while (1) {
        digitalWrite(23, val);
        if (val == 0) {
            val = 1;
        }
        else {
            val = 0;
        }
    }
    return 1;
}