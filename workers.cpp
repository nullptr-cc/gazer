#include <cerrno>
#include <cstring>
#include <csignal>
#include <unistd.h>

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <libconfig.h++>

using namespace std;

typedef struct {
    int pid;
    string command;
    string sout;
    string serr;
} worker_t;

typedef vector<worker_t*> workers_t;

workers_t workers;

int start_worker(const worker_t & worker)
{
    int child_pid = fork();

    if (child_pid) {
        return child_pid;
    }

    freopen(worker.sout.c_str(), "a", stdout);
    freopen(worker.serr.c_str(), "a", stderr);

    istringstream iss(worker.command);
    vector<string> tokens;
    copy(
        istream_iterator<string>(iss),
        istream_iterator<string>(),
        back_inserter<vector<string> >(tokens)
    );

    char * argv[10]; // FIXME: magic number

    for (short int i = 0; i < tokens.size(); ++i) {
        argv[i] = (char*) malloc(tokens[i].length() + 1);
        strcpy(argv[i], tokens[i].c_str());
    };

    argv[tokens.size()] = NULL;

    execvp(argv[0], argv);

    return -1;
}

void start_workers(const libconfig::Setting & programs)
{
    short int i, len = programs.getLength();

    for (i = 0; i < len; ++i) {
        const libconfig::Setting & prg = programs[i];
        if (prg.getType() != libconfig::Setting::TypeGroup) {
            continue;
        }

        worker_t* worker = new worker_t;
        worker->pid = 0;
        worker->sout = string("/dev/null");
        worker->serr = string("/dev/null");

        if (!prg.lookupValue("command", worker->command)) {
            std::cerr << "No command for " << prg.getName() << std::endl;
            continue;
        }

        prg.lookupValue("output", worker->sout);
        prg.lookupValue("errors", worker->serr);

        int worker_pid = start_worker(*worker);
        if (worker_pid == -1) {
            std::cerr << strerror(errno) << std::endl;
            continue;
        }

        worker->pid = worker_pid;
        workers.push_back(worker);

        std::cout << "Started worker \"" << prg.getName() << "\"" << std::endl;
    }
}

void check_workers()
{
    for (workers_t::iterator it = workers.begin(); it != workers.end(); ++it) {
        if (kill((*it)->pid, 0) == 0) {
            continue;
        }

        std::cerr << "Propblem with worker " << (*it)->pid << " - restarting" << std::endl;
        kill((*it)->pid, SIGKILL);

        (*it)->pid = start_worker(*(*it));
        if ((*it)->pid == -1) {
            std::cerr << strerror(errno) << std::endl;
            delete (*it);
            workers.erase(it);
        } else {
            std::cout << "New PID " << (*it)->pid << std::endl;
        }
    }
}
