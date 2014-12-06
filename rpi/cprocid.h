/*
 * procid.h
 *
 * A simple class to create a PID file
 *
 */

#ifndef _PROCESS_ID_H
#define _PROCESS_ID_H

#include <sys/types.h>
#include <unistd.h>

class cprocess_id {
	public:
		cprocess_id(const char *fn);
		~cprocess_id();
		bool file_exists();
		bool write_pid();
	private:
		char *filename;
		bool cf;
};

#endif
