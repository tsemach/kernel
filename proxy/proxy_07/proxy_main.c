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
		
	rc = read(fd, buff, sizeof(line));
	printf("read, rc = %i, buff is '%s;\n", rc, buff);

	rc = write(fd, line, sizeof(line));
	printf("write, rc = %i, sizeof(line) = %i\n", rc, sizeof(line));
		
	rc = read(fd, buff, sizeof(line));
	printf("read, rc = %i, buff is '%s;\n", rc, buff);

	// now get the statistic counters and reset them
	proxy_arg_t p;
	
	if (ioctl(fd, PROXY_GET_STATISTICS, &p) == -1) {
		perror("proxy_main: get statistic");

		return -1;
	}
	printf("prox_main: get cmd = 0x%x, nreads = %i, nwrites = %i\n", PROXY_GET_STATISTICS, p.nreads, p.nwrites);

	if (ioctl(fd, PROXY_CLR_STATISTICS, NULL) == -1) {
		perror("proxy_main: get statistic");

		return -1;
	}
	printf("prox_main: clear statistic is ok, cmd = 0x%x\n", PROXY_CLR_STATISTICS);

	if (ioctl(fd, PROXY_GET_STATISTICS, &p) == -1) {
		perror("proxy_main: get statistic");

		return -1;
	}
	printf("prox_main: get cmd = 0x%x, nreads = %i, nwrites = %i\n", PROXY_GET_STATISTICS, p.nreads, p.nwrites);


	close(fd);

	return 0;
}

