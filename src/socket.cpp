#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "../include/bstrwrap.h"
#include "../include/exceptions.hpp"
#include "../include/socket.hpp"

/* do everything needed to connect to an ip */
int cli_sock(int port, const char *ip)
{
        int fd;

        Bstrlib::CBString bip;

        bip = ip;

        /* options for socket (port, ip, and protocol) */
        struct sockaddr_in opt;
        opt.sin_family = AF_INET;
        opt.sin_port = htons(port);

        /* Get socket to write to */
        if (fd = socket(AF_INET, SOCK_STREAM, 0), fd < 0)
                throw sys_exception("socket");


        /* ip -> opt, turn localhost to the right ip */
        if (bip == "localhost")
                bip = "127.0.0.1";

        if (inet_pton(AF_INET, (const char *)bip, &opt.sin_addr) <= 0)
                throw sys_exception("inet_pton");

        /* connect with opt options */
        if (connect(fd, (struct sockaddr *)&opt, sizeof(opt)) < 0)
                throw sys_exception("Unable to connect to server.");

        return fd;
}

void cli_read(int fd, Bstrlib::CBString &input)
{
        int read_sz;            /* amount read */
        char buf[1024];         /* buffer for storing reads */
        while (read_sz = recv(fd, buf, sizeof(buf), MSG_DONTWAIT), read_sz > 0) /* read */
                input += buf;   /* concatenate buf onto the end of input */

        /*
         * If the loop ends and the error isn't EAGAIN (nothing to read)
         * throw an exception
         */
        if (errno != EAGAIN)
                throw sys_exception("Error reading from socket");
}
