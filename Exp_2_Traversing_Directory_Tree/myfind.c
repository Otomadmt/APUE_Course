#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "apue.h"
#include "error.c"
#include "pathalloc.c"

/* Function types which are called for each filename. */
typedef int myFindFunc(const char *, const struct stat *, int);

static myFindFunc findStat;
static myFindFunc findComp;
static myFindFunc findName;

static int myFTW(char *, myFindFunc *);
static int doPath(myFindFunc *);

static long nreg, nsreg, ndir, nblk, nslink, nchr,  nfifo, nsock, ntot;

/* Funcs and args used by findComp() and findName(). */
static long src_filesize, found_count;
static char *filebuf, *compbuf, *in_path, *in_filepath, *in_filename;
static void getAbsPath(const char *, char *);         /* Return the absolute path for given path. */
static int getFilenamePos(const char *);          /* Return the start position of filename in a path. */

int main(int argc, char *argv[]){

	int ret = 1, pathlen, pos;
	struct stat statbuf;

	/* Checking for given arguments. */
    if (!(argc == 2 || (argc == 4 && strncmp("-comp", argv[2], 5) == 0) || (argc >= 4 && strncmp("-name", argv[2], 5) == 0))){
        err_quit("wrong argument(s).\nusage:  myfind <pathname> [-comp <filename> | -name <str>...]\n");
    }

    /* Checking arg pathname. */
    if (lstat(argv[1], &statbuf) < 0){
        err_quit("lstat error: %s\n", argv[1]);
    }
    if (S_ISDIR(statbuf.st_mode) == 0){
    	/* Arg pathname is not a directory, changing it to current work dir. */
        strcpy(argv[1], ".");
    }

	if (argc == 2){
		ret = myFTW(argv[1], findStat);
	
		ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
		/* Setting ntot to 1 to avoid divided by 0. */
		if (ntot == 0){
			ntot = 1;
		}

		/* Report result. */
		printf("regular files  = %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
		printf("<=4096B in reg = %7ld, %5.2f %%\n", nsreg, nsreg*100.0/nreg);	
		printf("directories    = %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
		printf("block special  = %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
		printf("char special   = %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
		printf("FIFOs          = %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
		printf("symbolic links = %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
		printf("sockets        = %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
	}
	else if(argc == 4 && strncmp("-comp", argv[2], 5) == 0){
		/* Checking for arg filename. */
		if (lstat(argv[3], &statbuf) < 0){
            err_quit("lstat error: %s\n", argv[3]);
		}
        if (!S_ISREG(statbuf.st_mode)){
            err_quit("source file is not a regular one: %s\n", argv[3]);
        }

        /* Get info of source file. */
        int src_fd;
        src_filesize = statbuf.st_size;

        /* Load in source file content to buffer. */
        if ((src_fd = open(argv[3], O_RDONLY, FILE_MODE)) == -1){
            err_sys("can't open source file '%s'\n", argv[3]);
        }
        if ((filebuf = (char*)malloc(sizeof(char) * src_filesize)) == NULL ||
            (compbuf = (char*)malloc(sizeof(char) * src_filesize)) == NULL){
            err_sys("buffer malloc error\n");
    	}
        if (read(src_fd, filebuf, src_filesize) != src_filesize){
            err_sys("read error '%s'\n", argv[3]);
        }
        close(src_fd);

        /* Get the absolute path. */
      	in_path = path_alloc(&pathlen);
      	getAbsPath(argv[1], in_path);

      	char *in_dir;
        in_dir = path_alloc(&pathlen);
        pos = getFilenamePos(argv[3]);
        /* pos == 0 means that argv[3] leads to cur-work-dir.*/
        if (pos == 0){
            strcpy(in_dir, ".");
        }else{
            strncpy(in_dir, argv[3], pos);
        }
 
        in_filepath = path_alloc(&pathlen);
        getAbsPath(in_dir, in_filepath);
 
        /* Get the full path. */
        strcat(in_filepath, argv[3]+pos);

        /* Start searching. */
        printf("Matching content of '%s':", argv[3]);
        ret = myFTW(in_path, findComp);
        if (found_count == 0){
        	printf("No match.\n");
        }else{
        	printf("\n%ld match(es)found.\n", found_count);
        }
	}else
	{
        in_path = path_alloc(&pathlen);
        in_filename = path_alloc(&pathlen);
        printf("%d filename(s) to match:\n", argc - 3);
        /* Changing the input path to fullpath. */
        getAbsPath(argv[1], in_path);
 
		/* for-loop needed for dealing with multiple target filenames. */
		int i;
        for (i = 3; i < argc; i++){
            pos = getFilenamePos(argv[i]);
            /* Detect if args str contains any directory. */
            if (pos != 0 || strncmp(argv[i], "/", 1) == 0){
                printf("\nExpected file name, get directory: %s\n", argv[i]);
                continue;
            }
 
            strncpy(in_filename, argv[i], pathlen);
            found_count = 0;
            printf("\nMatching filename '%s'(%d/%d):", argv[i], i - 2, argc - 3);
            ret = myFTW(in_path, findName);
            if (found_count == 0){
            	printf("No match.\n");
            }else{
        		printf("\n%ld match(es) found.\n", found_count);
            }
        }
	}

	exit(ret);
}

#define	FTW_F	1		/* File other than directory. */
#define	FTW_D	2		/* Directory. */
#define	FTW_DNR	3		/* Directory that we can't read. */
#define	FTW_NS	4		/* file that we can't stat. */

static char *fullpath;

static int myFTW(char *pathname, myFindFunc *func)	/* Adopted from textbook. */
{
	int len;
	fullpath = path_alloc(&len);
	strncpy(fullpath, pathname, len);
	fullpath[len - 1] = 0;

	return(doPath(func));
}

static int doPath(myFindFunc *func)	/* Adopted from textbook with minor changes. */
{
	struct stat		statbuf;
	struct dirent	*dirp;
	DIR				*dp;
	int				ret;
	char			*ptr;

	if (lstat(fullpath, &statbuf) < 0)
		return(func(fullpath, &statbuf, FTW_NS));
	if (S_ISDIR(statbuf.st_mode) == 0)
		return(func(fullpath, &statbuf, FTW_F));

	if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
		return(ret);

	ptr = fullpath + strlen(fullpath);
	if(*(ptr-1) != '/'){
		*ptr++ = '/';
		*ptr = 0;
	}	/* It's better to avoid things like '//' in the path. */

	if ((dp = opendir(fullpath)) == NULL)
		return(func(fullpath, &statbuf, FTW_DNR));

	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0  ||
		    strcmp(dirp->d_name, "..") == 0)
				continue;

		strcpy(ptr, dirp->d_name);

		if ((ret = doPath(func)) != 0)
			break;
	}
	ptr[-1] = 0;

	if (closedir(dp) < 0)
		err_ret("can't close directory %s", fullpath);

	return(ret);
}

static void getAbsPath(const char *srcpath, char *abspath){

    int pathlen;
    char *dir, *ptr;
 
    dir = path_alloc(&pathlen);
    /* Record where I am. */
    if (getcwd(dir, pathlen) == NULL){
        err_sys("getcwd failed: %s\n", dir);
    }
 
    /* Get the path of where I came from. */
    if (chdir(srcpath) < 0){
        err_sys("chdir failed: %s\n", srcpath);
    }
    if (getcwd(abspath, pathlen) == NULL){
        err_sys("getcwd failed: %s\n", abspath);
    }

    /* Make sure the full path ends with '/'. */
    ptr = abspath + strlen(abspath);
    if (*(ptr-1) != '/'){
        *ptr++ = '/';
        *ptr = 0;
    }
 
    /* Get back to where I was. */
    if (chdir(dir) < 0){
        err_sys("chdir failed: %s\n", srcpath);
    }
 }

static int getFilenamePos(const char *pathname){

    int i, pos = 0;
    for (i = strlen(pathname)-1; i >= 0; i-- ){
        if (pathname[i] == '/'){
            pos = i;
            break;
        }
    }

    return (pos == 0) ? pos : pos + 1;
}

static int findStat(const char *pathname, const struct stat *statptr, int type)
{
	switch (type) {
	case FTW_F:
		switch (statptr->st_mode & S_IFMT) {
		case S_IFBLK:	nblk++;		break;
		case S_IFCHR:	nchr++;		break;
		case S_IFIFO:	nfifo++;	break;
		case S_IFLNK:	nslink++;	break;
		case S_IFSOCK:	nsock++;	break;
		case S_IFDIR:	break;
		case S_IFREG:	
			nreg++;
			if(statptr->st_size <= 4096 && statptr->st_size >= 0){
				nsreg++;
			}
			break;
		}
		break;
	case FTW_D:
		ndir++;
		break;
	case FTW_DNR:
	case FTW_NS:
		break;
	default:;
	}

	return(0);
}

static int findComp(const char *pathname, const struct stat *statptr, int type){

	if (type == FTW_F && (statptr->st_mode & S_IFMT) == S_IFREG && statptr->st_size == src_filesize){
        int fd;
        if ((fd = open(pathname, O_RDONLY, FILE_MODE)) == -1){
            return 0;
        }
        if (read(fd, compbuf, src_filesize) != src_filesize){
            err_sys("\nread error: '%s'\n", pathname);
        }
        close(fd);
 
        if (strcmp(filebuf, compbuf) == 0){
                /* Filtering the original one. */
                if (strcmp(in_filepath, pathname) != 0){
                    found_count++;
                    printf("\n%s", pathname);
                }
        }
    }
 
 	return 0;
}

static int findName(const char *pathname, const struct stat *statptr, int type){

	if (type == FTW_F){
        int pos;
        pos = getFilenamePos(pathname);
        if (strcmp(in_filename, pathname+pos) == 0){
            found_count++;
            printf("\n%s", pathname);
        }
    }
 
 	return 0;
}
