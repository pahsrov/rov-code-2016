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
#include "../include/args.hpp"


void loop(int sockfd, int jsfd, const struct js_layout &layout)
{
        try {
                struct js_event event;
                struct jsmath::motor_vals motors;
                Bstrlib::CBString input;

                jsmath::js_log js(jsfd);

                while (js_read(jsfd, event) != -1 && !js_quit(event, layout) ) {
                        jsmath::event_to_log(event, js);
                        jsmath::log_to_motors(motors, js, layout);
                        jsmath::send_motors(sockfd, motors);
                        /* jsmath::send_motors(0, motors); */
                        cli_read(sockfd, input);
                        puts(input);
                }
        } catch (std::exception &e) {
                fprintf(stderr, "\n%s\n", e.what());
        }

}

/* cleanup main */
int main(int argc, char **argv)
{
        int sockfd, jsfd;
        int port;
        FILE *config;
        struct js_layout layout;
        struct rov_opts opt;
        Bstrlib::CBString conf_path;

        conf_path = "joystick.conf";

        memset(&opt, 0, sizeof(opt));

        /* Ugly config mode, will replace with proper args later */
        try {
                handle_flags(argc, argv, opt);
        } catch (std::exception &e) {
                fprintf(stderr, "%s\n", e.what());
                return -1;
        }

        if (opt.conf_mode) {
                if (argc < 2) {
                        fprintf(stderr, "ERROR: No joystick path given\n");
                        return -1;
                }
                if (jsfd = open(argv[1], O_RDONLY), jsfd < 0) {
                        perror("open");
                        return -1;
                }

                config = fopen(conf_path, "w");
                js_config_mode(config, jsfd);
                return 0;
        }

        if (argc < 4) {
                fprintf(stderr, "Usage: %s JSPATH PORT IP\n", argv[0]);
                return 0;
        }

        /* get port from command line */
        port = atoi(argv[2]);

        /* open network connection */
        if (opt.use_stdout) {
                sockfd = 0;
        } else {
                try {
                        sockfd = cli_sock(port, argv[3]);
                } catch (std::exception &e) {
                        fprintf(stderr, "%s\n", e.what());
                        return -1;
                }
        }

        /* open joystick */
        if (jsfd = open(argv[1], O_RDONLY ), jsfd < 0) {
                perror("Unable to open joystick");
                return -1;
        }

        /* read config */
        config = fopen(conf_path, "r");
        if (!config) {
                perror("fopen");
                return -1;
        }

        try {
                js_load_config(config, layout);
        } catch (std::exception &e) {
                fprintf(stderr, "%s", e.what());
                return -1;
        }


        loop(sockfd, jsfd, layout);
        close(sockfd);
        close(jsfd);
        fclose(config);

        return 0;
}
