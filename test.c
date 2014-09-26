#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, int *argv[])
{
	int fd, saved_stdout;
	fd = creat(argv[1], 0644);
	printf("message before boksa\n");

	saved_stdout = dup(1);
	dup2(fd, 1);
	close(fd);

	printf("message after stdout dup \n");
	
	dup2(saved_stdout, 1);

	printf("message after stdout restore\n");
	return 0;
}
