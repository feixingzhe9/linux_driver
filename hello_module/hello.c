#include <linux/init.h>
#include <linux/module.h>

#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#define DRIVER_NAME "hello_driver"
#define DEVICE_NAME "hello"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Kaka");

static int hello_open(struct inode *inode, struct file *file){
    printk(KERN_EMERG "hello open ^_^\n");
    return 0;
}

//static int hello_write(struct inode *inode, struct file *file){
static int hello_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos){
    int i = 0;
    printk(KERN_EMERG "hello write ^_^\n");
    printk(KERN_EMERG "write data len: %d \n", (int)count);
    printk(KERN_EMERG "write data: ");
    for(; i < count; i++){
        printk(KERN_EMERG " %d ",buf[i]);
    }

    printk(KERN_EMERG "\n");
    return count;
}


//static ssize_t (*read) (struct file * filp, char __user * buffer, size_t size , loff_t * p); 
static ssize_t hello_read(struct file *filp,  char __user *buf,    size_t count, loff_t *ppos){
    int i = 0;
    char test_read[] = {'a', 'b', 'c', 'd', 'e', 'f', 'e', 'f', 'g', 'h', 'i', 'j', 'k'};
    printk(KERN_EMERG "hello read ^_^\n");
    if(count <= sizeof(test_read)){
        memcpy(buf, test_read, count); 
    }else{
        memcpy(buf, test_read, sizeof(test_read)); 
        count = sizeof(test_read); 
    }
    return count;
}

static int hello_release(struct inode *inode, struct file *file){
    printk(KERN_EMERG "hello release ^_^\n");
    return 0;
}

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    printk("cmd is %d, arg is %ld\n", cmd, arg);
    return 0;
}

static struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .read = hello_read,
    .write = hello_write,
    .release = hello_release,
    .unlocked_ioctl = hello_ioctl,
};

static struct miscdevice hello_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &hello_fops,
};

static int hello_probe(struct platform_device *pdv)
{
    printk(KERN_EMERG "hello probe ^_^\n");
    misc_register(&hello_dev);
    return 0;
}

static int hello_remove(struct platform_device *pdv)
{
    printk(KERN_EMERG "hello remove ^_^\n");
    misc_deregister(&hello_dev);
    return 0;
}

static void hello_shutdown(struct platform_device *pdv)
{
}

static int hello_suspend(struct platform_device *pdv, pm_message_t pmt)
{
    return 0;
}

static int hello_resume(struct platform_device *pdv)
{
    return 0;
}

static struct platform_driver hello_driver = {
    .probe = hello_probe,
    .remove = hello_remove,
    .shutdown = hello_shutdown,
    .suspend = hello_suspend,
    .resume = hello_resume,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    }
};


static int __init  hello_init(void)
{
    int driver_state;
    printk(KERN_EMERG "hello module has been mount!\n");
    driver_state = platform_driver_register(&hello_driver);
    printk(KERN_EMERG "platform_driver_register driver_state is %d\n", driver_state);
    platform_device_register_simple(DRIVER_NAME, -1, NULL, 0);
    printk(KERN_EMERG "platform_device_register_simple end\n");
    return 0;
}

static void  __exit hello_exit(void)
{
    printk(KERN_EMERG "hello module has been remove!\n");
    platform_driver_unregister(&hello_driver);
}

module_init(hello_init);
module_exit(hello_exit);

