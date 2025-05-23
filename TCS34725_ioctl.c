#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "TCS34725_ioctl.h"

extern struct i2c_client *tcs_client;  // your global client

static int tcs34725_read_color_component(struct i2c_client *client, int offset)
{
    u8 low = i2c_smbus_read_byte_data(client, TCS34725_COMMAND_BIT | offset);
    u8 high = i2c_smbus_read_byte_data(client, TCS34725_COMMAND_BIT | (offset + 1));
    return (high << 8) | low;
}

static int tcs34725_write_byte(struct i2c_client *client, u8 reg, u8 val)
{
    return i2c_smbus_write_byte_data(client, TCS34725_COMMAND_BIT | reg, val);
}

static int tcs34725_read_byte(struct i2c_client *client, u8 reg)
{
    return i2c_smbus_read_byte_data(client, TCS34725_COMMAND_BIT | reg);
}

long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value, ret;
    struct tcs34725_color_data color_data;
    u8 gain_val, atime_val, enable_val;

    if (!tcs_client)
        return -ENODEV;

    switch (cmd) {
    case TCS34725_IOCTL_READ_R:
        value = tcs34725_read_color_component(tcs_client, TCS34725_RDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_G:
        value = tcs34725_read_color_component(tcs_client, TCS34725_GDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_B:
        value = tcs34725_read_color_component(tcs_client, TCS34725_BDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_C:
        value = tcs34725_read_color_component(tcs_client, TCS34725_CDATAL);
        if (copy_to_user((int __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_READ_ALL:
        color_data.clear = tcs34725_read_color_component(tcs_client, TCS34725_CDATAL);
        color_data.red   = tcs34725_read_color_component(tcs_client, TCS34725_RDATAL);
        color_data.green = tcs34725_read_color_component(tcs_client, TCS34725_GDATAL);
        color_data.blue  = tcs34725_read_color_component(tcs_client, TCS34725_BDATAL);
        if (copy_to_user((struct tcs34725_color_data __user *)arg,
                         &color_data, sizeof(color_data)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_RESET:
        // Disable
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, 0x00);
        if (ret < 0) return ret;
        msleep(10);
        // PON
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, TCS34725_ENABLE_PON);
        if (ret < 0) return ret;
        msleep(10);
        // PON + AEN
        return tcs34725_write_byte(tcs_client, TCS34725_ENABLE,
                                   TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);

    case TCS34725_IOCTL_SET_GAIN:
        if (copy_from_user(&gain_val, (u8 __user *)arg, sizeof(gain_val)))
            return -EFAULT;
        return tcs34725_write_byte(tcs_client, TCS34725_CONTROL, gain_val);

    case TCS34725_IOCTL_SET_ATIME:
        if (copy_from_user(&atime_val, (u8 __user *)arg, sizeof(atime_val)))
            return -EFAULT;
        return tcs34725_write_byte(tcs_client, TCS34725_ATIME, atime_val);

    case TCS34725_IOCTL_ENABLE:
        if (copy_from_user(&enable_val, (u8 __user *)arg, sizeof(enable_val)))
            return -EFAULT;
        return tcs34725_write_byte(tcs_client, TCS34725_ENABLE, enable_val);

    case TCS34725_IOCTL_GET_STATUS:
        value = tcs34725_read_byte(tcs_client, TCS34725_ENABLE);
        if (copy_to_user((u8 __user *)arg, &value, sizeof(value)))
            return -EFAULT;
        return 0;

    case TCS34725_IOCTL_INIT:
        // 1) Disable
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, 0x00);
        if (ret < 0) return ret;
        msleep(10);
        // 2) PON
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE, TCS34725_ENABLE_PON);
        if (ret < 0) return ret;
        msleep(10);
        // 3) PON + AEN
        ret = tcs34725_write_byte(tcs_client, TCS34725_ENABLE,
                                  TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
        if (ret < 0) return ret;
        // 4) Gain = 1×
        ret = tcs34725_write_byte(tcs_client, TCS34725_CONTROL, 0x00);
        if (ret < 0) return ret;
        // 5) ATIME = 700 ms
        ret = tcs34725_write_byte(tcs_client, TCS34725_ATIME, 0x00);
        if (ret < 0) return ret;
        // 6) Delay 5 s
        msleep(5000);
        return 0;

    default:
        return -EINVAL;
    }
}
