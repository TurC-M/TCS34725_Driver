#ifndef TCS34725_IOCTL_H
#define TCS34725_IOCTL_H

#include <linux/fs.h>     
#include <linux/types.h>   

#define TCS34725_IOCTL_MAGIC 't'

// Các ioctl đọc dữ liệu màu
#define TCS34725_IOCTL_READ_R       _IOR(TCS34725_IOCTL_MAGIC, 0, int)
#define TCS34725_IOCTL_READ_G       _IOR(TCS34725_IOCTL_MAGIC, 1, int)
#define TCS34725_IOCTL_READ_B       _IOR(TCS34725_IOCTL_MAGIC, 2, int)
#define TCS34725_IOCTL_READ_C       _IOR(TCS34725_IOCTL_MAGIC, 3, int)
#define TCS34725_IOCTL_READ_ALL     _IOR(TCS34725_IOCTL_MAGIC, 4, struct tcs34725_color_data)

// Các ioctl điều khiển cảm biến
#define TCS34725_IOCTL_RESET        _IO(TCS34725_IOCTL_MAGIC, 5)
#define TCS34725_IOCTL_SET_GAIN     _IOW(TCS34725_IOCTL_MAGIC, 6, __u8)
#define TCS34725_IOCTL_SET_ATIME    _IOW(TCS34725_IOCTL_MAGIC, 7, __u8)
#define TCS34725_IOCTL_ENABLE       _IOW(TCS34725_IOCTL_MAGIC, 8, __u8)
#define TCS34725_IOCTL_GET_STATUS   _IOR(TCS34725_IOCTL_MAGIC, 9, __u8)

// Bổ sung ioctl để khởi tạo cảm biến (khởi động, config, đợi 5s)
#define TCS34725_IOCTL_INIT         _IO(TCS34725_IOCTL_MAGIC, 10)

// Các định nghĩa thanh ghi và bit điều khiển
#define TCS34725_COMMAND_BIT 0x80

#define TCS34725_ENABLE        0x00
#define TCS34725_ENABLE_PON    0x01
#define TCS34725_ENABLE_AEN    0x02

#define TCS34725_ATIME         0x01
#define TCS34725_CONTROL       0x0F

#define TCS34725_RDATAL        0x16
#define TCS34725_GDATAL        0x18
#define TCS34725_BDATAL        0x1A
#define TCS34725_CDATAL        0x14

// Cấu trúc lưu dữ liệu màu
struct tcs34725_color_data {
    int clear;
    int red;
    int green;
    int blue;
};

long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif 
