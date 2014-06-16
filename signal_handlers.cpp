#include <csignal>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

#include "main.h"

void sig_chld_handler(int sign)
{
    std::cout << "SIGCHLD - waiting" << std::endl;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void sig_int_handler(int sign)
{
    std::cout << "SIGINT" << std::endl;
    finalize();
}

void sig_term_handler(int sign)
{
    std::cout << "SIGTERM" << std::endl;
    finalize();
}

bool install_signal_handlers()
{
    signal(SIGCHLD, sig_chld_handler);
    signal(SIGINT, sig_int_handler);
    signal(SIGTERM, sig_term_handler);
}
