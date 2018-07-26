#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/fs.h>

#define CAR_STOP 0
#define CAR_FORWORLD 1
#define CAR_BACK 2
#define CAR_TURN_LIGHT 3
#define CAR_TURN_RIGHT 4

struct wheel {
    unsigned int	F_pin;
    unsigned int	B_pin;
    bool forworld;
    unsigned int	scl_is_output_only:1;
};

enum car_direction {
    CAR_DIR_STOP     = 0,
    CAR_DIR_FORWORLD = 1,
    CAR_DIR_BACK     = 2,
    CAR_DIR_RIGHT    = 3,
    CAR_DIR_LIGHT    = 4,
    CAR_DIR_NUM      = 5
};


struct bcm2835_car {
    enum car_direction car_dir;
    struct wheel car_wheel[4];
};

struct car_gpio_platform_data {
    struct miscdevice car_ctrl_miscdev;
    enum car_direction car_dir;
    struct wheel car_wheels[4];
    struct device car_dev;
};

/* set the direction of the pin */
static void car_gpio_set_direction(void *data, int num, int state)
{
    struct car_gpio_platform_data *pdata = data;
    struct wheel *car_wheel = pdata->car_wheels;

    if (state){
		gpio_direction_input(car_wheel[num].F_pin);
		gpio_direction_input(car_wheel[num].B_pin);
		printk("car-gpio set %d input \n",car_wheel[num].F_pin);
		printk("car-gpio set %d input \n",car_wheel[num].B_pin);
    }else{
		gpio_direction_output(car_wheel[num].F_pin, 0);
		gpio_direction_output(car_wheel[num].B_pin, 0);
		printk("car-gpio set %d output 0 \n",car_wheel[num].F_pin);
		printk("car-gpio set %d output 0 \n",car_wheel[num].B_pin);
    }
}


static void car_gpio_setwheel_direction(void *data, int num, int forworld)
{
    struct car_gpio_platform_data *pdata = data;
    struct wheel *car_wheel = pdata->car_wheels;

    if (1 == forworld){/*forworld*/
		gpio_set_value(car_wheel[num].F_pin, 1);
		gpio_set_value(car_wheel[num].B_pin, 0);
		printk("set gpio %d : 1\n",car_wheel[num].F_pin);
		printk("set gpio %d : 0\n",car_wheel[num].B_pin);
    }else if(-1 == forworld){ /*back*/
		gpio_set_value(car_wheel[num].F_pin, 0);
		gpio_set_value(car_wheel[num].B_pin, 1);
		printk("set gpio %d : 0\n",car_wheel[num].F_pin);
		printk("set gpio %d : 1\n",car_wheel[num].B_pin);
    }else { /*stop*/
		gpio_set_value(car_wheel[num].F_pin, 0);
		gpio_set_value(car_wheel[num].B_pin, 0);
		printk("set gpio %d : 0\n",car_wheel[num].F_pin);
		printk("set gpio %d : 0\n",car_wheel[num].B_pin);
    }

}

static void car_gpio_run_status(void *data, enum car_direction dir)
{
    struct car_gpio_platform_data *pdata = data;

    switch(dir){
	case CAR_DIR_STOP:
	    printk(KERN_DEBUG "CAR_DIR_STOP:\n");
	    car_gpio_setwheel_direction(pdata,0,0);
	    car_gpio_setwheel_direction(pdata,1,0);
	    car_gpio_setwheel_direction(pdata,2,0);
	    car_gpio_setwheel_direction(pdata,3,0);

	    break;
	case CAR_DIR_FORWORLD:
	    printk(KERN_DEBUG "CAR_DIR_FORWORLD:\n");
	    car_gpio_setwheel_direction(pdata,0,1);
	    car_gpio_setwheel_direction(pdata,1,1);
	    car_gpio_setwheel_direction(pdata,2,1);
	    car_gpio_setwheel_direction(pdata,3,1);
	    break;
	case CAR_DIR_BACK:
	    printk(KERN_DEBUG "CAR_DIR_BACK:\n");
	    car_gpio_setwheel_direction(pdata,0,-1);
	    car_gpio_setwheel_direction(pdata,1,-1);
	    car_gpio_setwheel_direction(pdata,2,-1);
	    car_gpio_setwheel_direction(pdata,3,-1);
	    break;
	case CAR_DIR_RIGHT:
	    printk(KERN_DEBUG "CAR_DIR_RIGHT:\n");

	    car_gpio_setwheel_direction(pdata,0,1);
	    car_gpio_setwheel_direction(pdata,1,-1);
	    car_gpio_setwheel_direction(pdata,2,1);
	    car_gpio_setwheel_direction(pdata,3,-1);
	    break;
	case CAR_DIR_LIGHT:
	    printk(KERN_DEBUG "CAR_DIR_LIGHT:\n");
	    car_gpio_setwheel_direction(pdata,0,-1);
	    car_gpio_setwheel_direction(pdata,1,1);
	    car_gpio_setwheel_direction(pdata,2,-1);
	    car_gpio_setwheel_direction(pdata,3,1);
	    break;
	
	default: break;

    }
}


static int of_car_gpio_get_pins(struct device_node *np,
	struct wheel *wheel_pin)
{
    int i=0;
    if (of_gpio_count(np) < 8)
	return -ENODEV;
    for(i=0; i<4; i++){
	wheel_pin[i].F_pin = of_get_gpio(np, i*2);
	wheel_pin[i].B_pin = of_get_gpio(np, i*2 + 1);
	printk(KERN_INFO "wheel_pin[i].F_pin = %d wheel_pin[i].B_pin=%d \n",wheel_pin[i].F_pin,wheel_pin[i].B_pin);
	if (wheel_pin[i].F_pin == -EPROBE_DEFER || wheel_pin[i].B_pin == -EPROBE_DEFER)
	    return -EPROBE_DEFER;

	if (!gpio_is_valid(wheel_pin[i].F_pin) || !gpio_is_valid(wheel_pin[i].B_pin)) {
	    pr_err("%s: invalid GPIO pins, F_pin=%d/B_pin=%d\n",
		    np->full_name, wheel_pin[i].F_pin, wheel_pin[i].F_pin);
	    return -ENODEV;
	}
    }
    return 0;
}

static int car_ctrl_open(struct inode *inode, struct file *file)
{
    struct car_gpio_platform_data *data;
    struct miscdevice *miscdev = file->private_data;

    data = container_of(miscdev,struct car_gpio_platform_data, car_ctrl_miscdev);

    file->private_data = data; /*Now, the private_data point to platform device driver data*/

    return 0;
}
static int car_ctrl_release(struct inode *inode, struct file *file)
{
    struct car_gpio_platform_data *data = file->private_data;

    kfree(data);
    return 0;
}

static long car_ctrl_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{

    struct car_gpio_platform_data *car_data = file->private_data; 
    if(cmd > CAR_DIR_STOP && cmd < CAR_DIR_NUM){
		printk("%s",__func__);	
		car_gpio_run_status(car_data,cmd);
	}

#if 0
    switch (cmd) {
	case CAR_FORWORLD:
	    printk(KERN_INFO "CAR_FORWORLD \n ");
	    car_gpio_run_status();
	    break;

	case CAR_BACK:
	    printk(KERN_INFO "CAR_BACK \n ");
	    break;

	case CAR_TURN_LIGHT:
	    printk(KERN_INFO "CAR_TURN_LIGHT \n ");
	    break;

	case CAR_TURN_RIGHT:
	    printk(KERN_INFO "CAR_TURN_RIGHT \n ");
	    break;

	case CAR_STOP:
	    printk(KERN_INFO "CAR_STOP \n ");
	    break;

	default:
	    err = -ENOTTY;
	    break;
    }
#endif
    return 1;

}

static const struct file_operations car_ctrl_fops = {
    .owner		= THIS_MODULE,
    .unlocked_ioctl	= car_ctrl_ioctl,
    .open		= car_ctrl_open,
    .release	= car_ctrl_release,
};

static int car_gpio_probe(struct platform_device *pdev)
{

    struct car_gpio_platform_data *pdata;
    struct wheel *car_wheel_temp;
    struct miscdevice *car_misc;
    unsigned int i;
    char name[20];
    int ret;
    pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);	
    car_wheel_temp = pdata->car_wheels;
    car_misc = &pdata->car_ctrl_miscdev;	
    printk(KERN_INFO "car_gpio_probe !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    /* First get the GPIO pins; if it fails, we'll defer the probe. */
    if (pdev->dev.of_node){
	ret = of_car_gpio_get_pins(pdev->dev.of_node,
		car_wheel_temp);
	if (ret)
	    return ret;
    } else {
	return -ENXIO;
    }

    for(i=0; i<4; i++) {
	printk(KERN_INFO "car_wheel_temp[i].F_pin = %d\n",car_wheel_temp[i].F_pin);	
	sprintf(name,"CAR_wheel_%d_F",i);
	ret = devm_gpio_request(&pdev->dev, car_wheel_temp[i].F_pin, name);
	if (ret) {
	    if (ret == -EINVAL)
		ret = -EPROBE_DEFER;	/* Try again later */
	    return ret;
	}
	printk(KERN_INFO "devm_gpio_request name = %s\n",name);	
	sprintf(name,"CAR_wheel_%d_B",i);
	ret = devm_gpio_request(&pdev->dev, car_wheel_temp[i].B_pin, name);
	if (ret) {
	    if (ret == -EINVAL)
		ret = -EPROBE_DEFER;	/* Try again later */
	    return ret;
	}
	printk(KERN_INFO "devm_gpio_request name = %s\n",name);	
    }


    for(i=0;i<4;i++)
	car_gpio_set_direction(pdata,i,0);
    //准备组册设备
    car_misc->minor = MISC_DYNAMIC_MINOR;
    car_misc->name = "car";
    car_misc->fops = &car_ctrl_fops;
    car_misc->parent = &pdev->dev;

    ret = misc_register(car_misc);
    if (ret) {
	dev_err(&pdev->dev,
		"unable to register misc device, err=%d\n", ret);
	return 0;
    }

    platform_set_drvdata(pdev, pdata);

    dev_info(&pdev->dev, "Finish car probe successfully \n");

    return 0;
}

static int car_ctrl_remove(struct platform_device *pdev)
{
    struct car_gpio_platform_data  *car_data;

    car_data = platform_get_drvdata(pdev);
    if (!car_data)
	return -ENODATA;

    misc_deregister(&car_data->car_ctrl_miscdev);
    return 0;
}
static int car_gpio_remove(struct platform_device *pdev)
{
    car_ctrl_remove(pdev);
	printk("car-gpio remove ...\n");
    return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id car_gpio_dt_ids[] = {
    { .compatible = "car-gpio", },
    { /*  */ }
};

MODULE_DEVICE_TABLE(of, car_gpio_dt_ids);
#endif

static struct platform_driver car_gpio_driver = {
    .driver		= {
	.name	= "car-gpio",
	.of_match_table	= of_match_ptr(car_gpio_dt_ids),
    },
    .probe		= car_gpio_probe,
    .remove		= car_gpio_remove,
};

static int __init car_gpio_init(void)
{
    int ret;

    printk(KERN_INFO "Entry car_gpio_init !!!!!!!!!!!!!!!!!!!!!!\n");
    ret = platform_driver_register(&car_gpio_driver);
    if (ret)
	printk(KERN_ERR "car-gpio: probe failed: %d\n", ret);

    return ret;
}
module_init(car_gpio_init);

static void __exit car_gpio_exit(void)
{
    platform_driver_unregister(&car_gpio_driver);
	printk("car-gpio exit . . .\n");
}
module_exit(car_gpio_exit);

MODULE_DESCRIPTION("Platform-independent car driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:car-gpio");

