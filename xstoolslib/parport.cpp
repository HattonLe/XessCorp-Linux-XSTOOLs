#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <linux/parport.h>

// Some code found on the web to test writing to /dev/parport0
// https://www.linuxquestions.org/questions/programming-9/imposible-to-send-data-to-dev-lp0-184885/

void TestParallelPortAccess()
{
    int fd, mode;
    int buff[1];

    if ((fd = open("/dev/parport0", O_RDWR)) < 0)
    {
        printf("Failed To open /dev device\n");
    }
    else
    {
        if (ioctl(fd, PPCLAIM) < 0)
        {
            printf("Failed to claim ParallelPort driver\n");
            perror("PPCLAIM");
            close(fd);
        }
        else
        {
            mode = IEEE1284_MODE_ECP;
            if (ioctl(fd, PPSETMODE, &mode) < 0)
            {
                printf("Failed to set ECP mode\n");
                perror("PPSETMODE");
                close(fd);
            }
            else
            {
                write(fd,"123",3);
                sleep(1);
                buff[0]=0;
                write(fd, buff,1);
                //dup2(1,fd);

                if (ioctl(fd, PPRELEASE) < 0)
                {
                    printf("Failed to unclaim ParallelPort driver\n");
                    close(fd);
                }
            }
        }
    }
}
