#include <string.h>
#include <sys/wait.h>
#include "../apue.2e/include/error.c"

#define _MAX_ARG_ 32
int
main(void)
{
	char	buf[MAXLINE];	// MAXLINE defined in apue.h
	pid_t	pid;			// child pid
	int		status;			// child returned status
	extern char **environ;	// system environment
	
	printf("%% "); // print prompt %
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		if (buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = 0;
		} // deal with the ending '\n' read by fgets()
		char *bufptr = strdup(buf);		// due to the strsep() will change the original string, make a dup

		if ((pid = fork()) < 0) {
			err_sys("fork error\n");
		} else if(pid == 0) { /* child */

			int i = 0;
			char *argv[_MAX_ARG_];			// storing arguments
			char *pathev = getenv("PATH");	// get $PATH to search for file
			
			if(pathev == NULL) {
				err_sys("can't get $PATH");
			}
			// divide input string using delimiter " "
			for(argv[i] = strsep(&bufptr, " "); argv[i] != NULL && i < _MAX_ARG_; argv[i] = strsep(&bufptr, " ")) {
				if(strcmp(argv[i], "")){
					i++;
				}
			}

			if(strstr(argv[0], "/") != NULL) {
				// path given in the argument, execute directly
				execve(argv[0], argv, environ);
			}else {
				// given no path, try to search in $PATH
				char *pathptr, path[100];
                pathptr = strsep(&pathev, ":"); 
                while (pathptr != NULL) { 
                    strcpy(path, pathptr);
                    strcat(path, "/");
                    strcat(path, argv[0]); 
                    execve(path, argv, environ);
                    pathptr = strsep(&pathev, ":");
                }
			}

			err_ret("exec fail:%s\n", buf);
			exit(127);
		}
		
		/* parent wait for signal from child */
		if ((pid = waitpid(pid, &status, 0)) < 0){
			err_sys("waitpid error\n");
		}
		free(bufptr);	//free the dup of buf
		printf("%% ");
	}
	printf("\n");
	exit(0);
}
