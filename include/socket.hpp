#pragma once
#include "bstrwrap.h"

int cli_sock(int port, const char *ip);
void cli_read(int fd, Bstrlib::CBString &buf);

