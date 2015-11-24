#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "../include/bstrwrap.h"
#include "../include/jsmath.hpp"
#include "../include/js.hpp"
#include "../include/exceptions.hpp"
#include "../include/socket.hpp"
#define JS_LONG_DEBUG

void enough_args(int argc, bool port = false, bool js = true)
{
        if (argc - optind - js - port - port < 0)
                throw_js_exception("Missing arg (expected %d) -- Use -h flag for help", js + port + port);
}

void loop(int sockfd, int jsfd, const struct js_layout &layout)
{
        struct js_event event;
        Bstrlib::CBString input;

        js_log js(jsfd);
        std::array<int, 6> motors;

        while (js_read(jsfd, event) != -1 && !js_quit(event, layout) ) {
                js.update(event);
                js.to_motors(layout, motors);
                send_motors(sockfd, motors);
                cli_read(sockfd, input);
        }

}

void print_help(const char *prog_name)
{
				fprintf(stderr,
                "Usage: %s [options] joy_path port ipaddr\n\n"
                "Options:\n"
                "\t-h\t\tPrint this help message and exit\n"
                "\t-s\t\tWrite to stdout instead of a socket\n"
                "\t-c conf_path\tUse a different config path than joystick.conf\n",
                prog_name);
}

void rov_main(int argc, char **argv)
{
        int sockfd = -1, jsfd;
        FILE *config;
        struct js_layout layout;
        Bstrlib::CBString conf_path = "joystick.conf";
        int opt;

        /* handle command line args */
        while (opt = getopt(argc, argv, "hsc:"), opt != -1) {
                switch(opt) {
                case 'h':
                        print_help(argv[0]);
                        exit(0);
                case 's':
                        sockfd = 0;
                        break;
                case 'c':
                        conf_path = optarg;
                        optind--;
                        break;
                case '?':
                        print_help(argv[0]);
                        exit(-1);
                }
        }
        enough_args(argc, sockfd);

        /* if no -s */
        if (sockfd) {
                int port;
                port = atoi(argv[argc - optind + 2]);
                const char *ip = argv[argc - optind + 3];
                sockfd = cli_sock(port, ip);
        }

        /* open joystick */
        if (jsfd = open(argv[argc - optind + 1], O_RDONLY ), jsfd < 0)
                throw_sys_exception("open %s", argv[argc - optind + 1]);

        /* read config */
        if (config = fopen(conf_path, "r"), !config)
                throw_sys_exception("fopen(%s)", (const char *)conf_path);
        js_load_config(config, layout);

        /* loop */
        loop(sockfd, jsfd, layout);

        close(sockfd);
        close(jsfd);
        fclose(config);
}

int main(int argc, char **argv)
{
        /* run our main in try catch block for easy error catching */
        try {
                rov_main(argc, argv);
        } catch (std::exception &e) {
                fprintf(stderr, "%s\n", e.what());
        } catch (...) {
                fprintf(stderr, "Caught unknown exception\n");
        }

        return 0;
}
