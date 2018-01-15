#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

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
	
	rc = write(fd, line, sizeof(line));
	printf("write, wc = %i, sizeof(line) = %i\n", rc, sizeof(line));
		
	rc = read(fd, buff, sizeof(line));
	printf("read, buff is '%s;\n", buff);

	close(fd);

	return 0;
}

