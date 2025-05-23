
# TCS34725 DRIVER 

## Project Overview

This project implements a Linux kernel driver for the TCS34725 color sensor using I2C communication. The driver allows interaction with the sensor through standard user-space APIs (`read`, `write`, and `ioctl`). The sensor measures the color channels (Clear, Red, Green, Blue) and provides data with a 16-bit resolution per channel, suitable for color detection applications in varying lighting conditions.

### Authors

- To Hoang Minh         –   22146037
- Diep The Nhat Minh    –   22146033
- Nguyen Chi Thanh      –   22146052


---

## Features

- 📥 **Sensor Data Interaction:** Read and configure color sensor data over I2C.
- 🔧 **Sensor Configuration:** Supports `ioctl` commands to enable the sensor, set integration time, and adjust gain.
- 🧪 **Sample Test Application:** Includes a user-space application for testing and interacting with the sensor.

---

## Project Structure

```
tcs34725-i2c-linux-driver/
├── src/
│   ├── TCS34725_main.c        # Main driver source
│   └── TCS34725_ioctl.c       # In out control
├── test/
│   └── test_driver.c          # User-space test application
└── README.md                  # Project documentation
```

---

## Setting Up the Driver

To get started with the TCS34725 driver:

1. **Compile the Driver:**

   ```bash
   make
   ```

2. **Load the Driver into the Kernel:**

   ```bash
   sudo insmod tcs34725.ko
   ```

3. **Verify Device Node Creation:**

   After loading the driver, check if the device node is created:

   ```bash
   ls -l /dev/TCS34725
   ```

---

---
## Main functin of sensor
| IOCTL Command              | Description                                                                                       |
| -------------------------- | ------------------------------------------------------------------------------------------------- |
| `TCS34725_IOCTL_INIT`      | **One-shot init**: reset sensor, set gain = 1×, integration time = 700 ms, enable, then wait 5 s. |
| `TCS34725_IOCTL_RESET`     | Reset sensor by disabling then re-enabling PON/AEN.                                               |
| `TCS34725_IOCTL_SET_GAIN`  | Set the ADC gain; in the test we use value 0x00 (1×).                                             |
| `TCS34725_IOCTL_SET_ATIME` | Set the integration time; in the test we use value 0x00 (700 ms).                                 |
| `TCS34725_IOCTL_READ_ALL`  | Read all four channels (clear, red, green, blue) at once, called every 100 ms.                    |

---
## Device Tree Configuration

To use the TCS34725 driver, add a sensor node to your Device Tree Source (DTS):

```dts
&i2c1 {
    tcs34725@29 {
        compatible = "taos,tcs34725";
        reg = <0x29>;
    };
};
```

This ensures proper configuration for the sensor to communicate over I2C.

---

## Driver API

The TCS34725 kernel module exposes a character device interface at `/dev/TCS34725`. Below are the supported file operations:

- `open()` / `release()` – Open or close the device file.
- `read()` – Read color data from the sensor.
- `write()` – Write log messages to the kernel (does not control hardware).
- `ioctl()` – Configure sensor settings like enable, integration time, and gain.

---

## Running the Test Application

1. **Compile the Test Application:**

   First, compile the user-space test application:

   ```bash
   gcc test_driver.c -o test_driver
   ```

2. **Run the Test:**

   After compiling, run the test application to interact with the sensor:

   ```bash
   sudo ./test_driver
   ```

---

## Video Demo
https://youtube.com/shorts/-mS88Uco_eU?feature=share
---

## Notes

- **I2C Interface:** Ensure that the I2C interface is enabled on your Raspberry Pi or embedded device.
  - You can enable I2C using `raspi-config` or manually.
  - To load the necessary I2C module, run:

    ```bash
    sudo modprobe i2c-dev
    ```

- **Check Device Creation:** If the device node `/dev/TCS34725` is not created after loading the driver, verify the following:
  - Check that the Device Tree overlay is applied correctly.
  - Use the `dmesg` command to view kernel logs for any error messages related to the sensor:

    ```bash
    dmesg | grep tcs34725
    ```

---

## Conclusion

This TCS34725 I2C Linux driver provides a straightforward method to interact with the TCS34725 color sensor on Linux-based platforms. It offers functionality for reading sensor data, configuring the sensor, and testing via a user-space application. The driver is suitable for embedded systems requiring accurate color detection in various environments.

---
