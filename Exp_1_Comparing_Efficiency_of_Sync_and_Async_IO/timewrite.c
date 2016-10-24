#include <sys/times.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

static void write_test(int, void *, size_t);
static void report_result(int, clock_t, struct tms *, struct tms *, int);

int main(int argc, char *argv[])
{
	off_t seek_res;
	size_t file_len = 0;
	int target_file;
	void *buf = NULL;

	//Check for given arguments
	if((argc != 2 && argc != 3) || (argc == 3 && (strncmp("sync", argv[2], 4) != 0)))
	{
		printf("argument(s) error\nusage:\ntimewrite <f1 f2\ntimewrite f1 sync <f2\n"); exit(1);
	}

	//Create new file
	if(argc == 2)
	{
		target_file = open(argv[1], O_WRONLY|O_TRUNC|O_CREAT);
	}else
	{
		target_file = open(argv[1], O_WRONLY|O_TRUNC|O_CREAT|O_SYNC);
		printf("[SYNC WRITE] ");
	}
	if(target_file == -1)
	{
		printf("cannot create new file\n"); exit(1);
	}

	//Read from STDIN to buffer
	if((seek_res = lseek(STDIN_FILENO, 0, 2)) == -1)
	{
		printf("STDIN_FILENO seek error\n"); exit(1);
	}else
	{
		lseek(STDIN_FILENO, 0, 0);
		file_len = seek_res;
		printf("File length: %zu\n",file_len);
		if((buf = malloc(file_len)) == NULL)
		{
			printf("malloc error\n"); exit(1);
		}else
		{
			if(read(STDIN_FILENO, buf, file_len) == -1)
			{
				printf("read error\n"); exit(1);
			}
		}
	}

	//Start write testing
	write_test(target_file, buf, file_len);

	//Exit normally
	exit(0);
	
}

static void write_test(int fd, void *src, size_t file_len)
{
	struct tms tms_start, tms_end;
	clock_t start, end;
	size_t loop_time, loop_count;
	int buffer_size;

	//Print table head
	printf("BUFFSIZE\tuser\t\tsys\t\treal\t\tloop\n");

	//Test write using buffer sized from 2^0 to 2^19
	for(buffer_size = 1; buffer_size <= 524288; buffer_size *= 2)
	{
		loop_time = file_len / buffer_size;
		//Set current file offset to 0
		if(lseek(fd, 0, 0) == -1)
		{
			printf("seek error while writing\n"); exit(1);
		}
		if((start = times(&tms_start)) == -1)
		{
			printf("times error\n"); exit(1);
		}
		for(loop_count = 0;loop_count < loop_time;loop_count++)
		{
			if(write(fd, (char *)src + (loop_count * buffer_size), buffer_size) != buffer_size)
			{
				printf("write error\n"); exit(1);
			}
		}
		if(write(fd, (char *)src + loop_time * buffer_size, file_len % buffer_size) != file_len % buffer_size)
		{
			printf("write error\n"); exit(1);
		}
		if((end = times(&tms_end)) == -1)
		{
			printf("times error\n"); exit(1);
		}
		if(file_len % buffer_size != 0)
		{
			loop_time++;
		}

		//Report result
		report_result(buffer_size, end-start, &tms_start, &tms_end, loop_time);
	}
}

static void report_result(int buffer_size, clock_t real, struct tms *tms_start, struct tms *tms_end, int loop_time)
{
	static long clktck = 0;

	//Get system clock ticks
	if(clktck == 0)
	{
		if((clktck = sysconf(_SC_CLK_TCK)) < 0)
		{
			printf("sysconf error\n"); exit(1);
		}
	}

	//Report result
	printf("%d\t\t%7.2f\t\t%7.2f\t\t%7.2f\t\t%d\n", buffer_size,
	 (tms_end->tms_utime - tms_start->tms_utime) / (double) clktck,
	 (tms_end->tms_stime - tms_start->tms_stime) / (double) clktck,
	 real / (double) clktck, loop_time);
}
