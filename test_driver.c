#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>        // For using uint8_t data type
#include "TCS34725_ioctl.h" // Custom ioctl commands and data structures for TCS34725

int main()
{
    // Open the device file for read/write access
    int fd = open("/dev/tcs34725", O_RDWR);
    if (fd < 0) {
        // Print error if device file can't be opened
        perror("Failed to open /dev/tcs34725");
        return 1; // Exit with error code
    }

    // Declare a struct variable to hold the color data read from sensor
    struct tcs34725_color_data color;
    int ret; // Variable to store return values of ioctl calls

    // Initialization step: perform sensor reset, configuration, enable sensor, and wait 5 seconds
    printf("Initializing sensor (reset, config, enable, wait 5s)...\n");
    ret = ioctl(fd, TCS34725_IOCTL_INIT);
    if (ret < 0) {
        // If initialization ioctl fails, print error and close file descriptor
        perror("TCS34725_IOCTL_INIT failed");
        close(fd);
        return 1;
    }
    printf("Initialization complete.\n");

    // Optional: reset the sensor again (can be skipped if init sets all config)
    printf("Resetting sensor again...\n");
    ret = ioctl(fd, TCS34725_IOCTL_RESET);
    if (ret < 0)
        perror("Reset failed");

    // Set sensor gain to 1x by sending a byte value of 0x00
    uint8_t gain = 0x00;  
    ret = ioctl(fd, TCS34725_IOCTL_SET_GAIN, &gain);
    if (ret < 0)
        perror("Set gain failed");

    // Set integration time (atime) to 700 ms by sending 0x00 (register value)
    uint8_t atime = 0x00;  
    ret = ioctl(fd, TCS34725_IOCTL_SET_ATIME, &atime);
    if (ret < 0)
        perror("Set integration time failed");

    // Begin an infinite loop to continuously read color data from the sensor
    printf("Starting color data reading:\n");
    while (1) {
        // Perform ioctl to read all color data (clear, red, green, blue) into 'color' struct
        ret = ioctl(fd, TCS34725_IOCTL_READ_ALL, &color);
        if (ret == 0) {
            // On success, print the values formatted nicely
            printf("Clear: %4d  Red: %4d  Green: %4d  Blue: %4d\n",
                   color.clear, color.red, color.green, color.blue);
        } else {
            // On failure, print error and break the loop
            perror("Failed to read color data");
            break;
        }
        usleep(100000); // Sleep 100 milliseconds before next reading
    }

    // Close the device file before exiting
    close(fd);
    return 0; // Exit normally
}
