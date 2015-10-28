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
        struct js_event event;
        struct jsmath::js_map js(jsfd);
        struct jsmath::js_send motors;
        Bstrlib::CBString input;

        try {
                jsmath::js_map js(jsfd);
        }
        while (js_read(jsfd, event) != -1 && !js_quit(event, layout) ) {
                try {
                        jsmath::read_to_map(js, event);
                        jsmath::map_to_send(motors, js, layout);
                        jsmath::sender(sockfd, motors, write);
                        /* cli_read(sockfd, input); */
                } catch (struct sys_exception &e) {
                        fprintf(stderr, "%s\n", e.what());
                        return;
                } catch (const char *msg) {
                        fprintf(stderr, "ERROR: %s\n", msg);
                        return;
                }
        }

        jsmath::free_input(js);
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
        } catch (CBStringException e) {
                fprintf(stderr, "Error: %s\n", e.what());
                return -1;
        } catch (sys_exception e) {
                fprintf(stderr, "Error: %s\n", e.what());
                return -1;
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
        js_load_config(config, layout);


        loop(sockfd, jsfd, layout);
        close(sockfd);
        close(jsfd);
        fclose(config);

        return 0;
}
