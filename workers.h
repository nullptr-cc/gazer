#ifndef __WORKERS_H
#define __WORKERS_H

#include <libconfig.h++>

bool start_workers(const libconfig::Setting &);
void check_workers();

#endif
