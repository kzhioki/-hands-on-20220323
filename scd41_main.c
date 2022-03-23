#include <sdk/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <nuttx/sensors/scd41.h>

int scd41_main(int argc, char *argv[])
{
  struct scd41_conv_data_s sdata;
  int fd;
  int ret;
  int i;

  printf("scd41 app is running.\n");

  fd = open("/dev/co2", O_RDWR);
  if (fd < 0)
    {
      printf("ERROR: open failed: %d\n", fd);
      return -1;
    }

  for (i = 0; i < 20; i++)
    {
      /* Sensor data is updated every 5 seconds. */

      sleep(5);

      ret = ioctl(fd, SNIOC_READ_CONVERT_DATA, (unsigned long)&sdata);
      if (ret < 0)
        {
          printf("Read error.\n");
          printf("Sensor reported error %d\n", ret);
        }
      else
        {
          printf("CO2[ppm]: %.2f, Temperature[C]: %.2f, RH[%%]: %.2f\n",
                 sdata.co2, sdata.temperature, sdata.humidity);
        }
    }

  ret = ioctl(fd, SNIOC_STOP, 0);
  if (ret < 0)
    {
      printf("Failed to stop: %d\n", errno);
    }

  close(fd);

  return 0;
}
