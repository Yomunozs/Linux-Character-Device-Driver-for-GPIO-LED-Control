#include <stdio.h>      // For printf, perror
#include <stdlib.h>     // For exit codes
#include <fcntl.h>      // For open
#include <unistd.h>     // For read, write, close
#include <string.h>     // For strcmp

#define DEVICE "/dev/led_driver"  // Device node created by the kernel module

/**
 * @brief Prints usage information to the user.
 */
void usage(const char *prog) {
    printf("Usage: %s [on|off|status]\n", prog);
}

/**
 * @brief Main function - communicates with the kernel driver to control or check the LED.
 */
int main(int argc, char *argv[]) {
    int fd;
    char buffer;

    // Ensure exactly one command-line argument
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    // Open the device file
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // Handle "on" command: write '1' to the device
    if (strcmp(argv[1], "on") == 0) {
        buffer = '1';
        write(fd, &buffer, 1);
    }
    // Handle "off" command: write '0' to the device
    else if (strcmp(argv[1], "off") == 0) {
        buffer = '0';
        write(fd, &buffer, 1);
    }
    // Handle "status" command: read from the device
    else if (strcmp(argv[1], "status") == 0) {
        read(fd, &buffer, 1);
        // Print human-readable LED status
        printf("LED is currently: %s\n", (buffer == 1) ? "ON" : "OFF");
    }
    // Invalid argument
    else {
        usage(argv[0]);
    }

    // Close the device file
    close(fd);
    return 0;
}
