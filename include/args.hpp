#pragma once
#include "bstrwrap.h"

struct rov_opts {
        int use_stdout;
        CBString conf_path;
        int conf_mode;
};

void print_help(const char *prog_name);

void handle_flags(int &argc, char **argv, struct rov_opts &opt);
