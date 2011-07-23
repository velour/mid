// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { Bufsz = 4096 };	// probably 1 disk block… if that matters

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: t <file>\n");
		exit(1);
	}

	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0)
		perror("Failed to open output file");

	for (;;) {
		char buf[Bufsz];
		ssize_t nrd = read(0, buf, Bufsz);
		if (nrd == 0)
			break;
		write(1, buf, nrd);
		write(fd, buf, nrd);
	}

	close(fd);
	return 0;
}