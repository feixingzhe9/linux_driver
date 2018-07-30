#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>


int fd;
int signal_come_flag = 0;

void sigintHandler(int sig)
{
    printf("killing on exit");
    
    signal_come_flag = 1;
    if(close(fd) < 0){
        printf("close %d error", fd); 
    }else{
        printf("close %d sucessfully", fd); 
    }
}

int main(int argc, char *argv[])
{
    printf("enter driver test %s %s \r\n", argv[1], argv[2]);
    char *led = "/dev/led_ctrl";
    char to_write[] = {1,2,3,4,5,6,7,8,9,0};
    char to_read[10] = {0};
    int get_read_size = 0;
    int i = 0;
    unsigned char on_off = 0;

    signal(SIGINT, sigintHandler);

    if((fd = open(led, O_RDWR | O_NOCTTY | O_NDELAY)) < 0){
        printf("open %s failed\n", led);
    }
    else{
        printf("%s fd is %d \r\n", led, fd);
        while(signal_come_flag == 0){
            printf("input ctrl:\n");
            scanf("%d",&on_off);
            printf("get input %d",on_off);
            //ioctl(fd, atoi(argv[1]), atoi(argv[2]));
            ioctl(fd, on_off, on_off);
        }

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
    if(close(fd) < 0){
        printf("close %d error", fd); 
    }else{
        printf("close %d sucessfully", fd); 
    }

    return 1;
}

