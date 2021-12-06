#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

int	main(int argc, char *argv[])
{
	int result = mkdir(argv[1], 0777);
	if (result == -1)
	{
		perror("damn");
	}
	else
		printf("create!\n");
}
