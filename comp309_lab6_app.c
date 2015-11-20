#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

int main()
{	
	char *dev_list[] = {"/dev/lab6_1","/dev/lab6_2","/dev/lab6_3","/dev/lab6_4","/dev/lab6_5","/dev/lab6_6","/dev/lab6_7","/dev/lab6_8"};
	int init_id,pid,fd;
	char current_values[40];
	init_id = getpid();
	fork();
	fork();
	fork();
	pid = getpid();
	pid = pid-init_id;

	if(fd = open(dev_list[pid],O_RDONLY)){
		printf("id %d open device successfully\n",pid);
	}
	else{
		exit(0);
	}
	for(;;){
		read(fd,current_values,sizeof(current_values));
		printf("id %d read successfully\n",pid);
	}
	close(fd);
	sleep(1);
}