#include <linux/init.h>      // Provides module_init and module_exit macros
#include <linux/module.h>    // Core header for Linux kernel modules
#include <linux/i2c.h>       // I2C subsystem support
#include <linux/fs.h>        // File operations for character devices
#include <linux/device.h>    // Device creation in sysfs
#include <linux/uaccess.h>   // copy_to_user, copy_from_user
#include <linux/delay.h>     // msleep()
#include "TCS34725_ioctl.h"  // IOCTL command definitions and register macros

#define DEVICE_NAME "tcs34725"      // Name of the device node (/dev/tcs34725)
#define CLASS_NAME  "tcs34725_class"// Name of the sysfs class (in /sys/class)

// Global pointer to the I2C client, set in probe(), used by ioctl handler
struct i2c_client *tcs_client;

// Kernel structures for character device registration
static struct class*  tcs_class  = NULL;
static struct device* tcs_device = NULL;
static int major_number;

//------------------------------------------------------------------------------
// File operations: open() and release() do nothing special
//------------------------------------------------------------------------------
static int tcs34725_open(struct inode *inodep, struct file *filep)
{
    // No special action required when device node is opened
    return 0;
}

static int tcs34725_release(struct inode *inodep, struct file *filep)
{
    // No special action required when device node is closed
    return 0;
}

//------------------------------------------------------------------------------
// File operations structure linking syscall to our handlers
//------------------------------------------------------------------------------
static struct file_operations fops = {
    .open           = tcs34725_open,       // Called on open()
    .release        = tcs34725_release,    // Called on close()
    .unlocked_ioctl = tcs34725_ioctl,      // Called on ioctl()
};

//------------------------------------------------------------------------------
// I2C probe: runs when driver is bound to the device
// 1) Register character device
// 2) Create sysfs class and device node
// 3) Initialize sensor over I2C
//------------------------------------------------------------------------------
static int tcs34725_probe(struct i2c_client *client)
{
    int ret;

    // Step 1: Allocate a major number dynamically for the character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "TCS34725: failed to register a major number\n");
        return major_number;
    }

    // Step 2: Create a device class under /sys/class/tcs34725_class
    tcs_class = class_create(CLASS_NAME);
    if (IS_ERR(tcs_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "TCS34725: failed to register device class\n");
        return PTR_ERR(tcs_class);
    }

    // Step 3: Create the device node /dev/tcs34725
    tcs_device = device_create(tcs_class, NULL,
        MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(tcs_device)) {
        class_destroy(tcs_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "TCS34725: failed to create the device\n");
        return PTR_ERR(tcs_device);
    }

    // Step 4: Sensor initialization sequence via I2C

    // 4a) Power ON (PON bit)
    ret = i2c_smbus_write_byte_data(client,
            TCS34725_COMMAND_BIT | TCS34725_ENABLE,
            TCS34725_ENABLE_PON);
    if (ret < 0) {
        printk(KERN_ERR "TCS34725: Failed to power on\n");
        return ret;
    }
    msleep(10); // Delay 10ms for power-up stabilization

    // 4b) Enable RGBC ADC (PON + AEN bits)
    ret = i2c_smbus_write_byte_data(client,
            TCS34725_COMMAND_BIT | TCS34725_ENABLE,
            TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    if (ret < 0) {
        printk(KERN_ERR "TCS34725: Failed to enable RGBC\n");
        return ret;
    }

    // 4c) Set integration time to 700ms (ATIME register = 0x00)
    ret = i2c_smbus_write_byte_data(client,
            TCS34725_COMMAND_BIT | TCS34725_ATIME,
            0x00);
    if (ret < 0) {
        printk(KERN_ERR "TCS34725: Failed to set integration time\n");
        return ret;
    }

    // 4d) Set gain to 1× (CONTROL register = 0x00)
    ret = i2c_smbus_write_byte_data(client,
            TCS34725_COMMAND_BIT | TCS34725_CONTROL,
            0x00);
    if (ret < 0) {
        printk(KERN_ERR "TCS34725: Failed to set gain\n");
        return ret;
    }

    // Store the I2C client pointer globally for ioctl usage
    tcs_client = client;

    // Wait one integration cycle (700ms) before first valid read
    msleep(700);

    printk(KERN_INFO "TCS34725 driver installed successfully\n");
    return 0;
}

//------------------------------------------------------------------------------
// I2C remove: runs when driver is unloaded or device removed
// Tears down device node and frees major number
//------------------------------------------------------------------------------
static void tcs34725_remove(struct i2c_client *client)
{
    // Remove the device from /dev
    device_destroy(tcs_class, MKDEV(major_number, 0));
    // Destroy the sysfs class
    class_destroy(tcs_class);
    // Unregister the character device
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "TCS34725: Driver removed\n");
}

//------------------------------------------------------------------------------
// I2C device ID table for matching devices to this driver
//------------------------------------------------------------------------------
static const struct i2c_device_id tcs34725_id[] = {
    { "tcs34725", 0 }, // Name used in device tree or board file
    { }
};
MODULE_DEVICE_TABLE(i2c, tcs34725_id);

//------------------------------------------------------------------------------
// Main I2C driver structure
//------------------------------------------------------------------------------
static struct i2c_driver tcs34725_driver = {
    .driver = {
        .name  = DEVICE_NAME, // Driver name
        .owner = THIS_MODULE, // Reference to this module
    },
    .probe    = tcs34725_probe,  // callback on matching device
    .remove   = tcs34725_remove, // callback on removal
    .id_table = tcs34725_id,     // Supported devices
};

//------------------------------------------------------------------------------
// Module init and exit
//------------------------------------------------------------------------------
static int __init tcs34725_init(void)
{
    printk(KERN_INFO "Initializing TCS34725 driver with IOCTL\n");
    // Register I2C driver with core
    return i2c_add_driver(&tcs34725_driver);
}

static void __exit tcs34725_exit(void)
{
    printk(KERN_INFO "Exiting TCS34725 driver\n");
    // Unregister I2C driver
    i2c_del_driver(&tcs34725_driver);
}

module_init(tcs34725_init);
module_exit(tcs34725_exit);

MODULE_AUTHOR("To Hoang Minh, Diep The Nhat Minh, Nguyen Chi Thanh");
MODULE_DESCRIPTION("TCS34725 I2C Driver with IOCTL Interface");
MODULE_LICENSE("GPL");
