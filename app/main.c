#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>

#define DEVICE "/dev/Akmal_virtual"

int fd = 0;
int ppos = 0;

int read_device() {
	int read_length = 0;
	ssize_t ret;
	char *data = (char *)malloc(1024 * sizeof(char));

	read_length = 500;

	memset(data, 0, sizeof(data));
	data[0] = '\0';

	ret = read(fd, data, read_length);
	
	printf("Kernel Response : %s", data);
	
	if (ret == -1) {
		free(data);
		return -1;
	}	else {
		free(data);
		return 0;
	}
}

int main()
{
  	char buff[500], user_command[500];

	fd = open(DEVICE, O_RDWR);

	read(fd, buff, 500);
	printf("Reading data from kernel: %s\n", buff);

	while(strcmp(user_command, "exit") != 0) {
		printf("Command: ");
		fgets(user_command, sizeof(user_command), stdin);
		
		write(fd, user_command, 13);
		read_device();
	}

	close(fd);

	return 0;
}
