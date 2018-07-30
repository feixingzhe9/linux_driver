//mpu6050.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
//#include <asm/uaccess.h>
#include <linux/uaccess.h>

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



struct mpu6050_pri {
    struct cdev dev;
    //struct miscdev dev;
    struct i2c_client *client;
};
struct mpu6050_pri dev;
static void mpu6050_write_byte(struct i2c_client *client,const unsigned char reg,const unsigned char val)
{ 
    char txbuf[2] = {reg,val};
    struct i2c_msg msg[2] = {
        [0] = {
            .addr = client->addr,
            .flags= W_FLG,
            .len = sizeof(txbuf),
            .buf = txbuf,
        },
    };
    i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
}
static char mpu6050_read_byte(struct i2c_client *client,const unsigned char reg)
{
    char txbuf[1] = {reg};
    char rxbuf[1] = {0};
    struct i2c_msg msg[2] = {
        [0] = {
            .addr = client->addr,
            .flags = W_FLG,
            .len = sizeof(txbuf),
            .buf = txbuf,
        },
        [1] = {
            .addr = client->addr,
            .flags = I2C_M_RD,
            .len = sizeof(rxbuf),
            .buf = rxbuf,
        },
    };

    i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
    return rxbuf[0];
}
static int dev_open(struct inode *ip, struct file *fp)
{
    return 0;
}
static int dev_release(struct inode *ip, struct file *fp)
{
    return 0;
}
static long dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    int res = 0;
    union mpu6050_data data = {{0}};
    switch(cmd){
        case GET_ACCEL:
            data.accel.x = mpu6050_read_byte(dev.client,ACCEL_XOUT_L);
            data.accel.x|= mpu6050_read_byte(dev.client,ACCEL_XOUT_H)<<8;
            data.accel.y = mpu6050_read_byte(dev.client,ACCEL_YOUT_L);
            data.accel.y|= mpu6050_read_byte(dev.client,ACCEL_YOUT_H)<<8;
            data.accel.z = mpu6050_read_byte(dev.client,ACCEL_ZOUT_L);
            data.accel.z|= mpu6050_read_byte(dev.client,ACCEL_ZOUT_H)<<8;
            break;
        case GET_GYRO:
            data.gyro.x = mpu6050_read_byte(dev.client,GYRO_XOUT_L);
            data.gyro.x|= mpu6050_read_byte(dev.client,GYRO_XOUT_H)<<8;
            data.gyro.y = mpu6050_read_byte(dev.client,GYRO_YOUT_L);
            data.gyro.y|= mpu6050_read_byte(dev.client,GYRO_YOUT_H)<<8;
            data.gyro.z = mpu6050_read_byte(dev.client,GYRO_ZOUT_L);
            data.gyro.z|= mpu6050_read_byte(dev.client,GYRO_ZOUT_H)<<8;
            printk("gyro:x %d, y:%d, z:%d\n",data.gyro.x,data.gyro.y,data.gyro.z);
            break;
        case GET_TEMP:
            data.temp = mpu6050_read_byte(dev.client,TEMP_OUT_L);
            data.temp|= mpu6050_read_byte(dev.client,TEMP_OUT_H)<<8;
            printk("temp: %d\n",data.temp);
            break;
        default:
            printk(KERN_INFO "invalid cmd");
            break;
    }
    printk("acc:x %d, y:%d, z:%d\n",data.accel.x,data.accel.y,data.accel.z);
    res = copy_to_user((void *)arg,&data,sizeof(data));
    return sizeof(data);
}

struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl, 
};

#define DEV_CNT 1
#define DEV_MI 0
#define DEV_MAME "mpu6050"

struct class *cls;
dev_t dev_no ;

static void mpu6050_init(struct i2c_client *client)
{
    mpu6050_write_byte(client, PWR_MGMT_1, 0x00);
    mpu6050_write_byte(client, SMPLRT_DIV, 0x07);
    mpu6050_write_byte(client, CONFIG, 0x06);
    mpu6050_write_byte(client, GYRO_CONFIG, 0x18);
    mpu6050_write_byte(client, ACCEL_CONFIG, 0x0);
}
static int mpu6050_probe(struct i2c_client * client, const struct i2c_device_id * id)
{
    dev.client = client;
    printk(KERN_INFO "%s\n", __func__);
    cdev_init(&dev.dev,&fops);

    alloc_chrdev_region(&dev_no,DEV_MI,DEV_CNT,DEV_MAME);

    cdev_add(&dev.dev,dev_no,DEV_CNT);

    mpu6050_init(client);

    /*自动创建设备文件*/
    cls = class_create(THIS_MODULE,DEV_MAME);
    device_create(cls,NULL,dev_no,NULL,"%s%d",DEV_MAME,DEV_MI);

    printk(KERN_INFO "probe\n");

    return 0;
}

static int mpu6050_remove(struct i2c_client * client)
{
    device_destroy(cls,dev_no);
    class_destroy(cls);
    unregister_chrdev_region(dev_no,DEV_CNT);
    printk(KERN_INFO "%s\n", __func__);
    return 0;
}

struct of_device_id mpu6050_dt_match[] = {
    {.compatible = "invensense,mpu6050"},
    {},
};

struct i2c_device_id mpu6050_dev_match[] = {};
struct i2c_driver mpu6050_driver = {
    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "mpu6050drv",
        .of_match_table = of_match_ptr(mpu6050_dt_match), 
    },
    .id_table = mpu6050_dev_match,
};
module_i2c_driver(mpu6050_driver);
MODULE_LICENSE("GPL");

