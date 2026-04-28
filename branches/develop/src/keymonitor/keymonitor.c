#include <stdio.h>
#include <stdbool.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/select.h>
#include <linux/input.h>
#include <time.h>

#define INADEQUATE_CONDITIONS 3
#define RESET_TIME 5 // seconds
#define DEFAULT_IP "192.168.2.100"
#define SED_IP_CMD                                                             \
    "sed -i '/^IP01\\+=/s/=.*/=" DEFAULT_IP "/' /mynand/config/BusLine.ini"
#define SED_MASK_CMD                                                           \
    "sed -i '/^SubNetMask01\\+=/s/=.*/=255.255.255.0/' "                       \
    "/mynand/config/BusLine.ini"

#define SED_GW_CMD                                                             \
    "sed -i '/^GateWay01\\+=/s/=.*/=192.168.2.2/' "                          \
    "/mynand/config/BusLine.ini"
/* Exit flag */
volatile bool g_quit = false;
void sig_handle(int arg)
{
    g_quit = true;
}

int main(int argc, char **argv)
{
    /* int c = 0; */
    /* int flag = 0; */
    char *dev = "/dev/input/event4"; /* Default device */
    time_t repeat_start = 0;
    bool is_repeating = false;

    /* Ctrl+c handler */
    signal(SIGINT, sig_handle);

    int fd = open(dev, O_RDONLY);
    if (fd < 0) {
        printf("Error: Failed to open device\n");
        return INADEQUATE_CONDITIONS;
    }
    printf("Please press the key to test.\n");

    while (!g_quit) {
        assert(fd >= 0);

        /* wait button being pressed or released. */
        fd_set input;
        FD_ZERO(&input);
        FD_SET(fd, &input);
        int ret = select(fd + 1, &input, NULL, NULL, NULL);
        if (ret < 0) {
            printf("%s", strerror(errno));
            continue;
        }

        /* read event */
        struct input_event buf;
        if (read(fd, &buf, sizeof(struct input_event)) < 0) {
            printf("%s", strerror(errno));
            return -1;
        }

        if (buf.code != 0) {
            if (buf.type == EV_KEY) {
                if (buf.value == 1) { // pressed
                    printf("code:%d pressed\n", buf.code);
                    is_repeating = false;
                }
                else if (buf.value == 0) { // released
                    printf("code:%d released\n", buf.code);
                    is_repeating = false;
                }
                else { // repeat
                    if (!is_repeating) {
                        repeat_start = time(NULL);
                        is_repeating = true;
                    }
                    else if (time(NULL) - repeat_start >= RESET_TIME) {
                        printf("code:%d reset\n", buf.code);
                        is_repeating = false;
                        printf("Executing interface reset script...\n");
                        char cmd[256];
                        // Set local network interface
                        snprintf(cmd, sizeof(cmd),
                                 "ifconfig eth0 %s netmask 255.255.255.0",
                                 DEFAULT_IP);
                        system(cmd);
                        // Update BusLine.ini IP01
                        system(SED_IP_CMD);
                        system(SED_MASK_CMD);
                        system(SED_GW_CMD);
                        system("sync");
                    }
                    else {
                        printf("code:%d repeat\n", buf.code);
                    }
                }
            }
        }
    }

    close(fd);
    return 0;
}
