#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

static void doCat(const char *path);
static void die(const char *s);

int main(int argc, char *argv[]) {
	int i;
	char endCharacter;
	if(argc<2) {
		fprintf(stderr,"%s file name has not given", argv[0]);
		exit(1);
	}
	for(int i=1; i<argc; i++) {
		doCat(argv[i]);
	}
	exit(0);

}

#define BUFFER_SIZE 4096

static void doCat(const char *path) {
	int fd;
	unsigned char buf[BUFFER_SIZE];
	int n;

	fd = open(path, O_RDONLY);
	if (fd < 0) die(path);

	while(1) {
		n = read (fd, buf, sizeof(buf));
		if(n < 0) die(path);
		if (!n) break;
		if(write(STDOUT_FILENO, buf, n) < 0) die(path);

	}
	if(close(fd) < 0) die(path);
}

static void die(const char *s) {
	perror(s);
	exit(1);
}
