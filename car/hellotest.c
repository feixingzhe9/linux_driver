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
    char *hello = "/dev/car";
    char to_write[] = {1,2,3,4,5,6,7,8,9,0};
    char to_read[10] = {0};
    int get_read_size = 0;
    int i = 0;

    if((fd = open(hello, O_RDWR | O_NOCTTY | O_NDELAY)) < 0){
        printf("open %s failed\n", hello);
    }
    else{
        printf("%s fd is %d \r\n", hello, fd);
        ioctl(fd, atoi(argv[1]), atoi(argv[2]));
		/*
        write(fd, to_write,sizeof(to_write));
        get_read_size = read(fd, to_read, sizeof(to_read));
        if(get_read_size > 0){
            if(get_read_size > sizeof(to_read)){
                printf("wtf");
                close(fd);
                return  -1;
            }
            printf("get read size : %d\n",get_read_size);
            for(i = 0; i < get_read_size; i++){
                printf(" %d ",to_read[i]);
            }
            printf("\n");
        }
		*/
    }
    close(fd);
    return 1;
}

