#include <stdio.h>
#include <sys/socket.h>
#include "../include/socket.hpp"
#include "../include/bstrwrap.h"

int main(int argc, char **argv)
{
        int fd;
        Bstrlib::CBString msg;

        if (argc < 3)
                return -1;

        fd = cli_sock(atoi(argv[1]), argv[2]);
        msg = "Test msg";

        send (fd, (const char *)msg, msg.length(), 0);
        return 0;
}
