#include <stdio.h>
#include <sys/socket.h>
#include "../include/socket.hpp"

int main(int argc, char **argv)
{
        int fd;

        if (argc < 3)
                return -1;

        fd = cli_sock(atoi(argv[1]), argv[2]);
        const char *msg = "Test msg";

        send (fd, (const char *)msg, sizeof(msg), 0);
        return 0;
}
