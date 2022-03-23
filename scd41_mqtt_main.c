#include <sdk/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <nuttx/sensors/scd41.h>
#include "MQTTClient.h"

int scd41_mqtt_main(int argc, char *argv[])
{
  struct scd41_conv_data_s sdata;
  int fd;
  int ret;
  int i;
	int rc = 0;
	unsigned char buf[100];
	unsigned char readbuf[100];
	int use_ssl = 0;
	MQTTSocket n;
	MQTTClient c;
	MQTTSocketInit(&n, use_ssl);
	MQTTSocketConnect(&n, "test.mosquitto.org", 1883);
	MQTTClientInit(&c, &n, 10000, buf, 100, readbuf, 100);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = "stdout-subscriber";
	data.username.cstring = NULL;
	data.password.cstring = NULL;

	data.keepAliveInterval = 10;
	data.cleansession = 1;
	printf("Connecting to %s %d\n", "test.mosquitto.org", 1883);
	
	rc = MQTTConnect(&c, &data);
	printf("Connected %d\n", rc);

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

          MQTTMessage pubmsg;

          memset(&pubmsg, 0, sizeof(pubmsg));
          pubmsg.qos = 0;
          pubmsg.retained = 0;
          pubmsg.dup = 0;

          char *topic;
          char str[16];
          snprintf(str, sizeof(str), "%.2f", sdata.co2);
          pubmsg.payload = str;
          pubmsg.payloadlen = strlen(str);
          topic = "test/spr/co2";
          printf("Publishing to %s\n", topic);
          rc = MQTTPublish(&c, topic, &pubmsg);
          printf("Published %d\n", rc);

          snprintf(str, sizeof(str), "%.2f", sdata.temperature);
          pubmsg.payload = str;
          pubmsg.payloadlen = strlen(str);
          topic = "test/spr/temp";
          printf("Publishing to %s\n", topic);
          rc = MQTTPublish(&c, topic, &pubmsg);
          printf("Published %d\n", rc);

          snprintf(str, sizeof(str), "%.2f", sdata.humidity);
          pubmsg.payload = str;
          pubmsg.payloadlen = strlen(str);
          topic = "test/spr/humi";
          printf("Publishing to %s\n", topic);
          rc = MQTTPublish(&c, topic, &pubmsg);
          printf("Published %d\n", rc);
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
