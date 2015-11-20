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
/* #include "../include/args.hpp" */

void enough_args(int argc)
{
        if (argc - optind == 2)
                throw_js_exception("Missing ip address");
        else if (argc - optind == 1)
                throw_js_exception("Missing port");
        else if (argc - optind == 0)
                throw_js_exception("Missing joystick path");
}


void loop(int sockfd, int jsfd, const struct js_layout &layout)
{
        struct js_event event;
        Bstrlib::CBString input;

        jsmath::js_log js(jsfd);
        std::array<int, 6> motors;

        while (js_read(jsfd, event) != -1 && !js_quit(event, layout) ) {
                js.update(event);
                js.to_motors(layout, motors);
                jsmath::send_motors(sockfd, motors);
                /* jsmath::send_motors(0, motors); */
                cli_read(sockfd, input);
                puts(input);
        }

}

void print_help(const char *prog_name)
{
				fprintf(stderr,
                "Usage: %s [options] joy_path port ipaddr\n\n"
                "Options:\n"
                "\t-h\t\tPrint this help message and exit\n"
                "\t-s\t\tWrite to stdout instead of a socket\n"
                "\t-C\t\tEnter config mode\n"
                "\t-c conf_path\tUse a different config path than usual (default: joystick.conf)\n"
                , prog_name);
}

void rov_main(int argc, char **argv)
{
        int sockfd = -1, jsfd;
        FILE *config;
        struct js_layout layout;
        Bstrlib::CBString conf_path;
        int opt;
        int conf_mode = 0;
        conf_path = "joystick.conf";

        /* handle opts */
        while (opt = getopt(argc, argv, "hsCc:"), opt != -1) {
                switch(opt) {
                case 'h':
                        print_help(argv[0]);
                        exit(0);
                case 's':
                        sockfd = 0;
                        break;
                case 'C':
                        conf_mode = 1;
                        break;
                case 'c':
                        conf_path = optarg;
                        break;
                case '?':
                        print_help(argv[0]);
                        exit(-1);
                }
        }

        if (conf_mode) {
                if (config = fopen(conf_path, "w"), !config)
                        throw_sys_exception("fopen %s", (const char *)conf_path);
                if (argc - optind == 0)
                        throw_js_exception("Missing js path");
                js_config_mode(config, argv[argc - optind + 1]);

                exit(0);
                fclose(config);
        }


        if (sockfd) {
                int port;
                enough_args(argc);

                port = atoi(argv[argc - optind + 2]); 
                const char *ip = argv[argc - optind + 3];
                sockfd = cli_sock(port, ip);
        } else {
                if (argc - optind == 0)
                        throw_js_exception("Missing js_path");
        }


        /* open joystick */
        if (jsfd = open(argv[argc - optind + 1], O_RDONLY ), jsfd < 0)
                throw_sys_exception("open %s", argv[argc - optind + 1]);

        /* read config */
        config = fopen(conf_path, "r");
        if (!config)
                throw_sys_exception("fopen");

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
