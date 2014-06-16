#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <libgen.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <libconfig.h++>

#include "daemonize.h"
#include "signal_handlers.h"
#include "workers.h"

using namespace std;
using namespace libconfig;

void finalize()
{
    std::cout << "Finalizing" << std::endl;
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    exit(EXIT_SUCCESS);
}

void load_config(std::string & path, Config & cfg)
{
    ostringstream err;

    char* cfg_rp = realpath(path.c_str(), NULL);

    if (cfg_rp == NULL) {
        err << "Cannot read file \"" << path << "\": " << strerror(errno);
        throw runtime_error(err.str());
    }

    FILE* fd = fopen(cfg_rp, "r");
    if (!fd) {
        err << "Cannot read file \"" << cfg_rp << "\": " << strerror(errno);
        throw runtime_error(err.str());
    }

    cfg.setIncludeDir(dirname(cfg_rp));

    try {
        cfg.read(fd);
        fclose(fd);
        free(cfg_rp);
    } catch(const FileIOException & fioex) {
        throw runtime_error("I/O error while reading config file");
    } catch(const ParseException & pex) {
        err << "Config parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
        throw runtime_error(err.str());
    }
}

int main(int argc, char* argv[])
{
    string config_path;

    if (argc < 2) {
        config_path = "/etc/gazer.conf";
    } else {
        config_path = argv[1];
    }

    Config cfg;

    try {
        load_config(config_path, cfg);
    } catch (const exception & e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    };

    bool cd;
    string log = "/dev/null";
    if (cfg.lookupValue("daemonize", cd) && cd) {
        cfg.lookupValue("log", log);
        if (!daemonize(log, log)) {
            return EXIT_FAILURE;
        }
    }

    install_signal_handlers();

    const Setting & programs = cfg.lookup("programs");

    start_workers(programs);

    while(true) {
        sleep(5);
        check_workers();
    }
}
