#include "../apue.2e/include/error.c"
#include "lock.h"
#include "lock.c"

#define _PHILO_NUM_ 5

static char* forks[5] = {"fork0", "fork1", "fork2", "fork3", "fork4"};
static int nsecs = 2;

void 
takeFork(int i) {
	if(i == _PHILO_NUM_ - 1) {
		/* Left hand first */
		lock(forks[0]);
		lock(forks[i]);
	} else {
		/* Right hand first */
		lock(forks[i]);
		lock(forks[i + 1]);
	}	
}

void 
putFork(int i) {
	if (i == _PHILO_NUM_ - 1) {
		/* Left hand first */
		unlock(forks[0]);
		unlock(forks[i]);
	} else {
		/* Right hand first */
		unlock(forks[i]);
		unlock(forks[i + 1]);
	}
}

void 
thinking(int i, int nsecs) {
	printf("philosopher %d is thinking\n", i);
	sleep(nsecs);
}

 void 
 eating(int i, int nsecs) {
	printf("philosopher %d is eating\n", i);
        sleep(nsecs);
 }

void philosopher(int i) {
	while (1) {
		thinking(i, nsecs);
		takeFork(i);
		eating(i, nsecs);
		putFork(i);
	}
}

int 
main(int argc, char *argv[]) {
	int i;
	pid_t pid;

	/* Check for argument */
	if (argc == 3 && strncmp("-t", argv[1], 2) == 0) {
		nsecs = atoi(argv[2]);
	} else if(argc != 1) {
		err_quit("usage: philosopher [-t <time>]");
	}

	/* Initiate forks */
	for(i = 0; i < _PHILO_NUM_; i++) {
		initlock(forks[i]);
	}

	/* Fork philosopher processes */
	for(i = 0; i < _PHILO_NUM_; i++) {
		//signal(SIGCHLD, SIG_IGN);	
		pid = fork();
		if(pid == 0) {
			philosopher(i);
		}
	}

	/* Wait for signal from children to avoid zombies */
	wait(NULL);

	return 0;
}
