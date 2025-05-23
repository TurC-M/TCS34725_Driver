#ifndef TCS34725_IOCTL_H
#define TCS34725_IOCTL_H

#include <linux/fs.h>      // Provides struct file
#include <linux/types.h>   // Provides kernel data types like __u8, __u16
#include <linux/ioctl.h>   // Provides _IO, _IOR, _IOW macros

// -----------------------------------------------------------------------------
// IOCTL command magic number
// -----------------------------------------------------------------------------
#define TCS34725_IOCTL_MAGIC 't'

// -----------------------------------------------------------------------------
// IOCTL commands for reading individual color channels (user-space read)
// _IOR: read from driver into user-space
// Arguments: pointer to int where the 16-bit color value will be stored
// -----------------------------------------------------------------------------
#define TCS34725_IOCTL_READ_R    _IOR(TCS34725_IOCTL_MAGIC, 0, int)  // Read red channel
#define TCS34725_IOCTL_READ_G    _IOR(TCS34725_IOCTL_MAGIC, 1, int)  // Read green channel
#define TCS34725_IOCTL_READ_B    _IOR(TCS34725_IOCTL_MAGIC, 2, int)  // Read blue channel
#define TCS34725_IOCTL_READ_C    _IOR(TCS34725_IOCTL_MAGIC, 3, int)  // Read clear channel

// Read all four channels in one call
// Argument: pointer to struct tcs34725_color_data in user-space
#define TCS34725_IOCTL_READ_ALL  _IOR(TCS34725_IOCTL_MAGIC, 4, struct tcs34725_color_data)

// -----------------------------------------------------------------------------
// IOCTL commands for sensor control (driver performs action, user-space sends args)
// _IO: simple command without data
// _IOW: write from user-space to driver
// -----------------------------------------------------------------------------
#define TCS34725_IOCTL_RESET        _IO(TCS34725_IOCTL_MAGIC, 5)      // Reset sensor (disable then re-enable)
#define TCS34725_IOCTL_SET_GAIN     _IOW(TCS34725_IOCTL_MAGIC, 6, __u8) // Set ADC gain (0x00..0x03)
#define TCS34725_IOCTL_SET_ATIME    _IOW(TCS34725_IOCTL_MAGIC, 7, __u8) // Set integration time (ATIME register)
#define TCS34725_IOCTL_ENABLE       _IOW(TCS34725_IOCTL_MAGIC, 8, __u8) // Enable/disable sensor manually
#define TCS34725_IOCTL_GET_STATUS   _IOR(TCS34725_IOCTL_MAGIC, 9, __u8) // Read back ENABLE register status

// -----------------------------------------------------------------------------
// One-shot initialization: full reset + config + wait 5 seconds for stabilization
// -----------------------------------------------------------------------------
#define TCS34725_IOCTL_INIT         _IO(TCS34725_IOCTL_MAGIC, 10)

// -----------------------------------------------------------------------------
// Sensor register addresses and bit masks
// -----------------------------------------------------------------------------
#define TCS34725_COMMAND_BIT 0x80  // Must OR this into register address for I2C command

#define TCS34725_ENABLE      0x00  // Enable register
#define TCS34725_ENABLE_PON  0x01  // Power ON bit
#define TCS34725_ENABLE_AEN  0x02  // RGBC enable bit

#define TCS34725_ATIME       0x01  // Integration time register
#define TCS34725_CONTROL     0x0F  // Gain control register

// Data registers (clear, red, green, blue low bytes)
#define TCS34725_CDATAL      0x14  // Clear data low byte
#define TCS34725_CDATAH      0x15  // Clear data high byte
#define TCS34725_RDATAL      0x16  // Red data low byte
#define TCS34725_RDATAH      0x17  // Red data high byte
#define TCS34725_GDATAL      0x18  // Green data low byte
#define TCS34725_GDATAH      0x19  // Green data high byte
#define TCS34725_BDATAL      0x1A  // Blue data low byte
#define TCS34725_BDATAH      0x1B  // Blue data high byte

// -----------------------------------------------------------------------------
// Structure used for bulk read (4 channels at once)
// -----------------------------------------------------------------------------
struct tcs34725_color_data {
    int clear;  // 16-bit clear channel value
    int red;    // 16-bit red channel value
    int green;  // 16-bit green channel value
    int blue;   // 16-bit blue channel value
};

// -----------------------------------------------------------------------------
// IOCTL function prototype
// Implemented in TCS34725_ioctl.c, called via unlocked_ioctl in file_operations
// -----------------------------------------------------------------------------
long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif // TCS34725_IOCTL_H
