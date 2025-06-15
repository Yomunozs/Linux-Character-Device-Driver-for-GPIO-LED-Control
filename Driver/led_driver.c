#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "led_driver"
#define CLASS_NAME  "led_class"
#define GPIO_LED    118  // PD22 On board

static dev_t dev_number;
static struct cdev led_cdev;
static struct class *led_class;
static struct device *led_device;

static char led_state = 0;

static int led_open(struct inode *inode, struct file *file)
{
    pr_info("[LED-DRV] Device opened\n");
    return 0;
}

static ssize_t led_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    pr_info("[LED-DRV] Reading state: %d\n", led_state);
    if (copy_to_user(buf, &led_state, 1))
        return -EFAULT;
    return 1;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    char value;
    if (copy_from_user(&value, buf, 1))
        return -EFAULT;

    pr_info("[LED-DRV] Writing value: %c\n", value);

    if (value == '1') {
        gpio_set_value(GPIO_LED, 1);
        led_state = 1;
    } else if (value == '0') {
        gpio_set_value(GPIO_LED, 0);
        led_state = 0;
    } else {
        pr_warn("[LED-DRV] Invalid input: %c\n", value);
        return -EINVAL;
    }

    return len;
}

static int led_release(struct inode *inode, struct file *file)
{
    pr_info("[LED-DRV] Device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = led_open,
    .read    = led_read,
    .write   = led_write,
    .release = led_release,
};

static int __init led_init(void)
{
    int ret;

    pr_info("[LED-DRV] Initializing LED driver\n");

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("[LED-DRV] Failed to allocate char device region\n");
        return ret;
    }

    cdev_init(&led_cdev, &fops);
    ret = cdev_add(&led_cdev, dev_number, 1);
    if (ret < 0) {
        pr_err("[LED-DRV] Failed to add cdev\n");
        goto unregister_region;
    }

    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        pr_err("[LED-DRV] Failed to create class\n");
        ret = PTR_ERR(led_class);
        goto del_cdev;
    }

    led_device = device_create(led_class, NULL, dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        pr_err("[LED-DRV] Failed to create device\n");
        ret = PTR_ERR(led_device);
        goto destroy_class;
    }

    ret = gpio_request(GPIO_LED, "led_gpio");
    if (ret) {
        pr_err("[LED-DRV] Failed to request GPIO %d\n", GPIO_LED);
        goto destroy_device;
    }

    ret = gpio_direction_output(GPIO_LED, 0);
    if (ret) {
        pr_err("[LED-DRV] Failed to set GPIO direction\n");
        goto free_gpio;
    }

    pr_info("[LED-DRV] Driver initialized successfully\n");
    return 0;

free_gpio:
    gpio_free(GPIO_LED);
destroy_device:
    device_destroy(led_class, dev_number);
destroy_class:
    class_destroy(led_class);
del_cdev:
    cdev_del(&led_cdev);
unregister_region:
    unregister_chrdev_region(dev_number, 1);
    return ret;
}

static void __exit led_exit(void)
{
    pr_info("[LED-DRV] Exiting driver\n");

    gpio_set_value(GPIO_LED, 0);
    gpio_free(GPIO_LED);
    device_destroy(led_class, dev_number);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("[LED-DRV] Driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Based on Johannes4Linux, updated for Lichee RV Dock");
MODULE_DESCRIPTION("Character device LED driver with GPIO control and debug logs");
