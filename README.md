# EmbeddedLinux-GPIO-LED-Driver

This project provides a character device driver for controlling an on-board LED via a GPIO pin on the Lichee RV Dock running Embedded Linux. It includes a kernel module and a user-space command-line interface for interacting with the LED device.


---

# 🏗️ Directory Structure

    ├── led_driver.c     # Kernel module for LED control
    ├── Makefile         # Makefile for building kernel module
    ├── led_control.c    # User-space tool to interact with the device
    ├── Makefile         # Makefile for building the user tool
    └── README.md        # This documentation
 
🛠️ Build

# Build the kernel module
    > cd /home/root/gpio_led_driver/driver
    > make
# Build the userspace app
    > cd /home/root/gpio_led_driver/user_app
    > make

📦 Load Module
    sudo insmod led_driver.ko

📦 Check Logs
    > dmesg | tail -20

📦 Verify Device Registered
    > cat /proc/devices | grep led_driver

📦 Create Device Node
    > mknod /dev/led_driver c <major> 0
    > chmod 666 /dev/led_driver

📦 Test the Driver
    > ./led_control on
    > ./led_control off
    > ./led_control status

📦 Check Kernel Logs
    > dmesg | tail

🧼 Unload Module

    sudo rmmod led_driver

---

## Codebase Overview

### 1. Kernel Module: `led_driver.c`

This file implements a Linux kernel character device that controls an LED connected to GPIO pin **PD22 (GPIO 118)**.

#### Core Features:

1. **Character Device Registration**
   - Allocates a major/minor number
   - Registers file operations (open, read, write, release)

2. **GPIO Management**
   - Requests and sets GPIO direction as output
   - Writes 0 or 1 to turn the LED off or on

3. **Device Interface**
   - `/dev/led_driver` is created dynamically using udev

4. **I/O Behavior**
   - `write()` accepts `'1'` or `'0'` to toggle the LED
   - `read()` returns current LED state (`1` or `0`)

5. **Debug Logging**
   - Uses `pr_info()` and `pr_warn()` for runtime diagnostics (visible in `dmesg`)

#### Module Initialization Flow:

```c
module_init(led_init);
module_exit(led_exit);
```

This ensures the GPIO and device file are created and removed cleanly.

#### Example Kernel Log Output:

```bash
[LED-DRV] Initializing LED driver
[LED-DRV] Writing value: 1
[LED-DRV] Reading state: 1
[LED-DRV] Exiting driver
```

---

### 2. Kernel Module Makefile

**Makefile**

```makefile
obj-m += gpio_led.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
```

To compile:

```bash
make
```

To clean:

```bash
make clean
```

---

### 3. User-Space CLI Tool: `led_control.c`

This user-level application communicates with `/dev/led_driver` to control or query the LED.

#### Command Usage:

```bash
./led_control on      # Turns the LED ON
./led_control off     # Turns the LED OFF
./led_control status  # Prints current LED state
```

#### CLI Behavior:

- Uses `open()`, `write()`, `read()` syscalls.
- Prints human-readable output for `status` mode.
- Validates arguments, prints usage on error.

#### Sample Output:

```bash
$ ./led_control status
LED is currently: ON
```

---

### 4. User-Space CLI Makefile

**Makefile**

```makefile
all:
	gcc -o led_control led_control.c

clean:
	rm -f led_control
```

Compile it with:

```bash
make
```

Clean it with:

```bash
make clean
```

---

## Testing Expectations

1. **Driver Load**
   - Load with `insmod gpio_led.ko`
   - Validate with `dmesg | grep LED-DRV`

2. **Device File**
   - Check for `/dev/led_driver`
   - If not present, create manually with:
     ```bash
     sudo mknod /dev/led_driver c <major> 0
     ```

3. **User Interaction**
   - Run `./led_control on`, `off`, or `status`
   - Observe the LED and verify with `dmesg`

4. **Driver Unload**
   - Use `rmmod gpio_led`
   - Confirm cleanup in logs and `/dev` disappearance

---