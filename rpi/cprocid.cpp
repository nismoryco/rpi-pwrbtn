/*
 * procid.h
 *
 * A simple class to create a PID file
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cprocid.h"

cprocess_id::cprocess_id(const char *fn)
{
	filename = strdup(fn);
	cf = false;
}

cprocess_id::~cprocess_id()
{
	if (cf == true)
		unlink(filename);
	free(filename);
}

bool cprocess_id::file_exists()
{
	int fd;
	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return false;
	close(fd);
	return true;
}

bool cprocess_id::write_pid()
{
	int fd;
	ssize_t bytes_written;
	char buf[10];
	if (file_exists() == true)
		return false;
	fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1)
		return false;
	snprintf(buf, sizeof(buf), "%d", getpid());
	bytes_written = write(fd, buf, strlen(buf));
	close(fd);
	if (bytes_written < (ssize_t)strlen(buf)) {
		unlink(filename);
		return false;
	}
	cf = true;
	return true;
}

