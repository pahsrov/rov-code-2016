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
                        jsmath::send_motors(sockfd, motors, 0);
                        jsmath::send_motors(stdout, motors);
                        /* cli_read(sockfd, input); */
                }
        } catch (std::exception &e) {
                fprintf(stderr, "\n%s\n", e.what());
        }

}

int main(int argc, char **argv)
{
        int sockfd, jsfd;
        int port;
        FILE *config;
        struct js_layout layout;

        /* Ugly config mode, will replace with proper args later */
        if (argc > 2) {
                if (!strcmp(argv[1], "--config-mode")) {
                        config = fopen("./joystick.conf", "w");
                        jsfd = open(argv[2], O_RDONLY | O_NONBLOCK);

                        if (!config || jsfd < 0) {
                                perror("open/fopen");
                                return -1;
                        }
                        js_config_mode(config, jsfd);

                        return 0;
                }
        }

        if (argc < 4) {
                fprintf(stderr, "Usage: %s JSPATH PORT IP\n", argv[0]);
                return 0;
        }

        /* get port from command line */
        port = atoi(argv[2]);

        /* open network connection */
        try {
                sockfd = cli_sock(port, argv[3]);
        } catch (std::exception &e) {
                fprintf(stderr, "%s", e.what());
        }
        /* open joystick */
        if (jsfd = open(argv[1], O_RDONLY | O_NONBLOCK), jsfd < 0) {
                perror("Unable to open joystick");
                return -1;
        }

        /* read config */
        config = fopen("../joystick.conf", "a+");
        if (!config) {
                perror("fopen");
                return -1;
        }

        try {
                js_load_config(config, layout);
        } catch (std::exception e) {
                fprintf(stderr, "%s", e.what());
        }


        loop(sockfd, jsfd, layout);
        close(sockfd);
        close(jsfd);
        fclose(config);

        return 0;
}
