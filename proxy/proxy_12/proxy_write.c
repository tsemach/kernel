#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include "proxy_ioctl.h"

int
main(int argc, char *argv[])
{
	int fd = open("/dev/proxy", O_RDWR);
	if (fd < 0) {
		printf("error openning /dev/proxy\n");

		return -1;
	}
	printf("successful open /dev/proxy deriver\n");

	int rc;
	const char line[] = {"hello proxy, this is user space"};
	char buff[1024] = {0};

	// write / read serveral times
	rc = write(fd, line, sizeof(line));
	printf("write, rc = %i, sizeof(line) = %i\n", rc, sizeof(line));
		
	close(fd);

	return 0;
}

