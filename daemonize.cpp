#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <iostream>

#include "daemonize.h"

using std::string;

bool daemonize(string sout = "/dev/null", string serr = "/dev/null")
{
    int childpid;

    if ((childpid = fork()) == -1) {
        std::cerr << "An error occurred while daemonizing: " << strerror(errno) << std::endl;
        return false;
    }

    if (childpid) {
        exit(EXIT_SUCCESS);
    }

    std::cout << "Becoming a daemon, PID = " << getpid() << std::endl;
    return detach_from_terminal(sout, serr);
}

bool detach_from_terminal(string sout, string serr)
{
	if (setsid() == -1) {
        std::cerr << "setsid(): " << strerror(errno) << std::endl;
		return false;
	}

	if (freopen("/dev/null", "r", stdin) == NULL) {
        std::cerr << "Unable to replace stdin with /dev/null: " << strerror(errno) << std::endl;
        return false;
	}

	if (freopen(sout.c_str(), "w", stdout) == NULL) {
		std::cerr << "Unable to replace stdout with " << sout << ": " << strerror(errno) << std::endl;
		return false;
	}

	if (freopen(serr.c_str(), "w", stderr) == NULL) {
		std::cerr << "Unable to replace stderr with " << serr << ": " << strerror(errno) << std::endl;
		return false;
	}

    return true;
}

