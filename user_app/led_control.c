#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/led_driver"

void usage(const char *prog) {
    printf("Usage: %s [on|off|status]\n", prog);
}

int main(int argc, char *argv[]) {
    int fd;
    char buffer;

    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    if (strcmp(argv[1], "on") == 0) {
        buffer = '1';
        write(fd, &buffer, 1);
    } else if (strcmp(argv[1], "off") == 0) {
        buffer = '0';
        write(fd, &buffer, 1);
    } else if (strcmp(argv[1], "status") == 0) {
        read(fd, &buffer, 1);
        printf("LED is currently: %s\n", (buffer == 1) ? "ON" : "OFF");
    } else {
        usage(argv[0]);
    }

    close(fd);
    return 0;
}
