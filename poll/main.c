#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>

#define DEVICE_NAME "/dev/char_dev"

int main()
{
    int fd;
    char buf[32];
    struct pollfd fds[1];
    int ret;

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("Failed to open the device!\n");
        return -1;
    }

    printf("Enter a message to send to the device:\n");
    scanf("%[^\n]%*c", buf);

    write(fd, buf, sizeof(buf));

    fds[0].fd = fd;
    fds[0].events = POLLIN | POLLPRI | POLLERR;
    fds[0].revents = 0;

    printf("Waiting for data to be available...\n");

    ret = poll(fds, 1, 5000); // Wait for 5 seconds
    if (ret < 0)
    {
        printf("Polling failed!\n");
        close(fd);
        return -1;
    }
    else if (ret == 0)
    {
        printf("Timeout!\n");
        close(fd);
        return -1;
    }
    else
    {
        if (fds[0].revents & POLLIN)
        {
            read(fd, buf, sizeof(buf));
            printf("Received message from the device: %s\n", buf);
        }
    }

    close(fd);
    return 0;
}
