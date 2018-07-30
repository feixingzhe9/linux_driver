#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>


//mpu6050_common.h
#define MPU6050_MAGIC 'K'

union mpu6050_data
{
    struct {
        short x;
        short y;
        short z;
    }accel;
    struct {
        short x;
        short y;
        short z;
    }gyro;
    unsigned short temp;
};

#define GET_ACCEL _IOR(MPU6050_MAGIC, 0, union mpu6050_data)
#define GET_GYRO  _IOR(MPU6050_MAGIC, 1, union mpu6050_data) 
#define GET_TEMP  _IOR(MPU6050_MAGIC, 2, union mpu6050_data)





//mpu6050_drv.h

#define SMPLRT_DIV      0x19    //陀螺仪采样率，典型值：0x07(125Hz)
#define CONFIG          0x1A    //低通滤波频率，典型值：0x06(5Hz)
#define GYRO_CONFIG     0x1B    //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define ACCEL_CONFIG        0x1C    //加速计自检、测量范围及高通滤波，典型值：0x18(不自检，2G，5Hz)
#define ACCEL_XOUT_H        0x3B
#define ACCEL_XOUT_L        0x3C
#define ACCEL_YOUT_H        0x3D
#define ACCEL_YOUT_L        0x3E
#define ACCEL_ZOUT_H        0x3F
#define ACCEL_ZOUT_L        0x40
#define TEMP_OUT_H      0x41
#define TEMP_OUT_L      0x42
#define GYRO_XOUT_H     0x43
#define GYRO_XOUT_L     0x44
#define GYRO_YOUT_H     0x45
#define GYRO_YOUT_L     0x46
#define GYRO_ZOUT_H     0x47    //陀螺仪z轴角速度数据寄存器（高位）
#define GYRO_ZOUT_L     0x48    //陀螺仪z轴角速度数据寄存器（低位）
#define PWR_MGMT_1      0x6B    //电源管理，典型值：0x00(正常启用)
#define WHO_AM_I        0x75    //IIC地址寄存器(默认数值0x68，只读)
#define SlaveAddress        0x68    //MPU6050-I2C地址寄存器
#define W_FLG           0
#define R_FLG           1






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



int main(int argc, char * const argv[])
{
    int fd = open("/dev/mpu60500",O_RDWR);

    signal(SIGINT, sigintHandler);

    if(-1== fd){
        perror("open");
        return -1;
    }
    union mpu6050_data data = {{0}};
    while(signal_come_flag == 0){
        ioctl(fd,GET_ACCEL,&data);
        printf("acc:x %d, y:%d, z:%d\n",data.accel.x,data.accel.y,data.accel.z);
        ioctl(fd,GET_GYRO,&data);
        printf("gyro:x %d, y:%d, z:%d\n",data.gyro.x,data.gyro.y,data.gyro.z);
        ioctl(fd,GET_TEMP,&data);
        printf("temp: %d\n",data.temp);
        sleep(1);
    }
    return 0;
}
