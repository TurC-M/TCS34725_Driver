#include <linux/uaccess.h>   // copy_to_user, copy_from_user
#include <linux/i2c.h>       // I2C bus helper functions
#include <linux/kernel.h>    // core kernel functions and macros
#include <linux/delay.h>     // msleep()
#include "TCS34725_ioctl.h"  // IOCTL command definitions and register macros

extern struct i2c_client *tcs_client;  // Global I2C client set in probe()

/**
 * Read a 16-bit color component from the sensor by reading two consecutive bytes.
 * @client: I2C client handle
 * @offset: Register offset for the low byte of the channel
 * Returns combined 16-bit value (high<<8 | low)
 */
static int tcs34725_read_color_component(struct i2c_client *client, int offset)
{
    u8 low, high;

    // Read low-order byte from (COMMAND_BIT | offset)
    low  = i2c_smbus_read_byte_data(client, TCS34725_COMMAND_BIT | offset);
    // Read high-order byte from the next register
    high = i2c_smbus_read_byte_data(client, TCS34725_COMMAND_BIT | (offset + 1));
    // Combine into a single 16-bit value
    return (high << 8) | low;
}

/**
 * Write a single byte to a sensor register.
 * @client: I2C client handle
 * @reg: Register offset (without COMMAND_BIT)
 * @val: Value to write
 * Returns 0 on success or negative error code.
 */
static int tcs34725_write_byte(struct i2c_client *client, u8 reg, u8 val)
{
    // COMMAND_BIT (0x80) tells the sensor this is a register access
    return i2c_smbus_write_byte_data(client, TCS34725_COMMAND_BIT | reg, val);
}

/**
 * Read a single byte from a sensor register.
 * @client: I2C client handle
 * @reg: Register offset
 * Returns byte value (0–255) or negative error code.
 */
static int tcs34725_read_byte(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, TCS34725_COMMAND_BIT | reg);
}

/**
 * Main IOCTL dispatch function.
 * Called by user-space via unlocked_ioctl.
 * @file: file object (unused here)
 * @cmd: IOCTL command code
 * @arg: user-space pointer or immediate argument
 */
long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value, ret;
    struct tcs34725_color_data color_data;
    u8 gain_val, atime_val, enable_val;

    // Ensure probe() has set tcs_client
    if (!tcs_client)
        return -ENODEV;

    switch (cmd) {

    /***** Single-channel reads (Red, Green, Blue, Clear) *****/
    case TCS34725_IOCTL_READ_R:
        // Read Red channel
        value = tcs34725_read_color_component(tcs_client, TCS34725_RDATAL);
        // Copy result back to user-space
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_G:
        // Read Green channel
        value = tcs34725_read_color_component(tcs_client, TCS34725_GDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_B:
        // Read Blue channel
        value = tcs34725_read_color_component(tcs_client, TCS34725_BDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_C:
        // Read Clear channel (ambient light)
        value = tcs34725_read_color_component(tcs_client, TCS34725_CDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    /***** Bulk read: all four channels at once *****/
    case TCS34725_IOCTL_READ_ALL:
        // Fill struct with all channel readings
        color_data.clear = tcs34725_read_color_component(tcs_client, TCS34725_CDATAL);
        color_data.red   = tcs34725_read_color_component(tcs_client, TCS34725_RDATAL);
        color_data.green = tcs34725_read_color_component(tcs_client, TCS34725_GDATAL);
        color_data.blue  = tcs34725_read_color_component(tcs_client, TCS34725_BDATAL);
        // Copy struct back to user-space
        if (copy_to_user((struct tcs34725_color_data __user *)arg,
                         &color_data, sizeof(color_data)))
            return -EFAULT;
        return 0;

    /***** Sensor reset: disable then re-enable PON and AEN *****/
    case TCS34725_IOCTL_RESET:
        // 1) Disable all
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, 0x00);
        if (ret < 0) return ret;
        msleep(10);
        // 2) Power ON only
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, TCS34725_ENABLE_PON);
        if (ret < 0) return ret;
        msleep(10);
        // 3) Power ON + RGBC enable
        return tcs34725_write_byte(tcs34725_client, TCS34725_ENABLE,
                                   TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);

    /***** Set sensor gain (0x00..0x03) *****/
    case TCS34725_IOCTL_SET_GAIN:
        // Copy desired gain from user-space
        if (copy_from_user(&gain_val, (u8 __user *)arg, sizeof(gain_val)))
            return -EFAULT;
        // Write to CONTROL register
        return tcs34725_write_byte(tcs_client, TCS34725_CONTROL, gain_val);

    /***** Set integration time (ATIME register) *****/
    case TCS34725_IOCTL_SET_ATIME:
        if (copy_from_user(&atime_val, (u8 __user *)arg, sizeof(atime_val)))
            return -EFAULT;
        return tcs34725_write_byte(tcs_client, TCS34725_ATIME, atime_val);

    /***** Manual enable/disable the sensor *****/
    case TCS34725_IOCTL_ENABLE:
        if (copy_from_user(&enable_val, (u8 __user *)arg, sizeof(enable_val)))
            return -EFAULT;
        return tcs34725_write_byte(tcs_client, TCS34725_ENABLE, enable_val);

    /***** Read back the ENABLE register status *****/
    case TCS34725_IOCTL_GET_STATUS:
        value = tcs34725_read_byte(tcs_client, TCS34725_ENABLE);
        if (copy_to_user((u8 __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    /***** One-shot initialization: full sequence + 5s delay *****/
    case TCS34725_IOCTL_INIT:
        // 1) Disable sensor
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, 0x00);
        if (ret < 0) return ret;
        msleep(10);
        // 2) Power ON
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, TCS34725_ENABLE_PON);
        if (ret < 0) return ret;
        msleep(10);
        // 3) Enable RGBC
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE,
                                  TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
        if (ret < 0) return ret;
        // 4) Set default gain = 1×
        ret = tcs34725_write_byte(tcs34725_client, TCS34725_CONTROL, 0x00);
        if (ret < 0) return ret;
        // 5) Set default integration time = 700 ms
        ret = tcs34725_write_byte(tcs34725_client, TCS34725_ATIME, 0x00);
        if (ret < 0) return ret;
        // 6) Wait 5000 ms for first valid measurement
        msleep(5000);
        return 0;

    default:
        // Unknown ioctl command
        return -EINVAL;
    }
}
