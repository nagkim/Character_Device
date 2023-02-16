#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

#define BUFFER_LENGTH 4096               
static char receive[BUFFER_LENGTH];     
 
int main()
{
        int fd;
        int32_t value, number;
 
        printf("\nStarting device test...\n");
        fd = open("/dev/char_dev", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return errno;
        }
 
        printf("Enter the Value to send: ");
        scanf("%d",&number);
        printf("Writing Value to Driver...\n");
        ioctl(fd, WR_VALUE, (int32_t*) &number); 
 
        ioctl(fd, RD_VALUE, (int32_t*) &value);
        printf("Reading value is :  %d\n", value);
 
        printf("End of the test.\n");
        close(fd);
}