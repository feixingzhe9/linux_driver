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

struct led_pins_t{
    unsigned int pin;
};

struct led_platform_data_t {
    struct miscdevice led_ctrl_miscdev;
    struct led_pins_t led_pins;
    struct device led_dev;
};


static void led_gpio_set_direction(void *data, int state)
{
    struct led_pins_t *led_pins_tmp = data;

    if(state){
        gpio_direction_output(led_pins_tmp->pin, 1);
        printk("set gpio %d output 1 \n", led_pins_tmp->pin);
    }else{
        gpio_direction_output(led_pins_tmp->pin, 0);
        printk("set gpio %d output 0 \n", led_pins_tmp->pin);
    }
}

static void led_on_off(void *data, bool on_off)
{
    struct led_platform_data_t *pdata = data;
    struct led_pins_t *led_pin = &pdata->led_pins;

    if(true == on_off){
        gpio_set_value(led_pin->pin, 1);
        printk("set led gpio %d : 1 \n",led_pin->pin);
    }else {
        gpio_set_value(led_pin->pin, 0);
        printk("set led gpio %d : 0 \n",led_pin->pin);
    }
}

static int of_led_get_pins(struct device_node *np, struct led_pins_t *pins)
{
    int err = 0;
    if(pins == NULL){
        printk(KERN_ERR " %s: parameter ERROR \n",__func__);
    }
    if(of_gpio_count(np) < 1){
        err = -ENODEV;
        printk(KERN_ERR "%s: of_gpio_count < 1 ! ! ! \n", __func__);
        goto exit;
    }
    pins->pin = of_get_gpio(np,0);
    printk(KERN_INFO "%s:  get gpio : %d \n", __func__,pins->pin);
    if(!gpio_is_valid(pins->pin)){
        printk(KERN_ERR "gpio %d is unvalid ! ! \n",pins->pin); 
        err = -ENODEV; 
        goto exit;
    }

exit:
    return err;
}

static int led_ctrl_open(struct inode *inode, struct file *file)
{
    struct led_platform_data_t *data;
    struct miscdevice *miscdev = file->private_data;

    printk("%s: start . . \n",__func__);
    data = container_of(miscdev, struct led_platform_data_t, led_ctrl_miscdev);
    //data = container_of(inode->i_cdev, struct led_platform_data_t, led_ctrl_miscdev);

    file->private_data = data;  /*Now, the private_data point to platform device driver data*/

    printk("%s: end \n",__func__);
    

    return 0;
}

static int led_ctrl_release(struct inode *inode, struct file *file)
{
    struct led_platform_data_t *data = file->private_data;

    kfree(data);
    
    printk("%s \n",__func__);

    return 0;
}


static long led_ctrl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct led_platform_data_t *led_data = file->private_data;

    if(cmd > 0){
        printk("%s: start to set led  1 \n", __func__);
        led_on_off(led_data, 1);
    }else{
        printk("%s: start to set led  0 \n", __func__);
        led_on_off(led_data, 0);
    }

    return 1;
}


static const struct file_operations led_ctrl_fops = {
    .owner		= THIS_MODULE,
    .unlocked_ioctl	= led_ctrl_ioctl,
    .open		= led_ctrl_open,
    .release	= led_ctrl_release,
};

static int led_gpio_probe(struct platform_device *pdev)
{
    struct led_pins_t *led_pins_tmp;
    struct miscdevice *led_misc;   
    struct led_platform_data_t *pdata;
    char name[20];
    int ret;

    printk("%s \n",__func__);
    pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);	

    led_pins_tmp = &pdata->led_pins;
    led_misc = &pdata->led_ctrl_miscdev;	

    /* First get the GPIO pins; if it fails, we'll defer the probe. */
    if (pdev->dev.of_node){
        printk("%s: pdev->dev.of_node is not NULL \n", __func__); 
        ret = of_led_get_pins(pdev->dev.of_node, led_pins_tmp);
        if (ret){
            return ret;
        }
    } else {
        printk(KERN_ERR "%s: pdev->dev.of_node is NULL ! ! ! \n", __func__); 
        return -ENXIO;
    }

    printk(KERN_INFO "led_pin = %d\n",led_pins_tmp->pin);	
    sprintf(name,"led_ctrl_gpio");
    ret = devm_gpio_request(&pdev->dev, led_pins_tmp->pin, name);
    if (ret) {
        if (ret == -EINVAL){
            ret = -EPROBE_DEFER;	/* Try again later */
        }
        return ret;
    }
    printk(KERN_INFO "devm_gpio_request name = %s\n",name);	

    //car_gpio_set_direction(pdata,i,0);
    led_gpio_set_direction((void *)led_pins_tmp, 0);

    //准备组册设备
    led_misc->minor = MISC_DYNAMIC_MINOR;
    led_misc->name = "led_ctrl";
    led_misc->fops = &led_ctrl_fops;        //todo 
    led_misc->parent = &pdev->dev;

    ret = misc_register(led_misc);
    if (ret) {
        dev_err(&pdev->dev, "unable to register misc device, err=%d\n", ret);
        return 0;
    }

    platform_set_drvdata(pdev, pdata);

    dev_info(&pdev->dev, "Finish led probe successfully \n");

    return 0;
}


static int led_ctrl_remove(struct platform_device *pdev)
{
    struct led_platform_data_t *led_data;

    printk("%s \n ",__func__);

    led_data = platform_get_drvdata(pdev);
    if(!led_data){
        return -ENODATA;
    }

    misc_deregister(&led_data->led_ctrl_miscdev);

    printk("%s: successfull \n",__func__);
    return 0;
}

static int led_gpio_remove(struct platform_device *pdev)
{
    led_ctrl_remove(pdev);
	printk("led-gpio remove ...\n");
    return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id led_gpio_dt_ids[] = {
    { .compatible = "led_gpio", },
    { /*  */ }
};

MODULE_DEVICE_TABLE(of, led_gpio_dt_ids);
#endif

static struct platform_driver led_gpio_driver = {
    .driver		= {
	.name	= "led_gpio",
	.of_match_table	= of_match_ptr(led_gpio_dt_ids),
    },
    .probe		= led_gpio_probe,
    .remove		= led_gpio_remove,
};

static int __init led_gpio_init(void)
{
    int ret;

    printk(KERN_INFO "Entry led_gpio_init !!!!!!!!!!!!!!!!!!!!!!\n");
    ret = platform_driver_register(&led_gpio_driver);
    if (ret){
		printk(KERN_ERR "led-gpio: probe failed: %d\n", ret);
	}

    printk("%s: successfull \n",__func__);
    return ret;
}
module_init(led_gpio_init);

static void __exit led_gpio_exit(void)
{
    platform_driver_unregister(&led_gpio_driver);
	printk("led-gpio exit . . .\n");
}
module_exit(led_gpio_exit);

MODULE_DESCRIPTION("led control driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("led_ctrl");

