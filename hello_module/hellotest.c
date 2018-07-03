#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>


int main(int argc, char *argv[])
{
    int fd;
    printf("enter driver test %s %s \r\n", argv[1], argv[2]);
    char *hello = "/dev/hello";

    if((fd = open(hello, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    {
        printf("open %s failed\n", hello);
    }
    else
    {
        printf("%s fd is %d \r\n", hello, fd);
        ioctl(fd, atoi(argv[1]), atoi(argv[2]));
    }
    close(fd);
    return 1;
}

