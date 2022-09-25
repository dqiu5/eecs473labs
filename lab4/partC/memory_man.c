
#include <stdio.h>

int main() {
    FILE *fp;
    char buff[10];

    fp = fopen("/dev/memory", "w+");
    fprintf(fp, "honey badger");
    fclose(fp);
    fp = fopen("/dev/memory", "r");
    fgets(buff, 10, fp);
    printf(buff);
    fclose(fp);
    return 1;
}