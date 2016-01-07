#include "../include/js.hpp"
#include <unistd.h>
#include <fcntl.h>

void js_write_config(FILE *config, const struct js_layout &layout)
{
        /* function to print to the config file */
        auto confprint = [=](const char *var, int val)
                {
                        if (fprintf(config, "%s = %d\n", var, val) < 0)
                                throw "fprintf";
                };

        confprint("x_ax", layout.x_ax);
        confprint("y_ax", layout.y_ax);
        confprint("z_ax", layout.z_ax);
        confprint("rot_ax", layout.rot_ax);
        confprint("quit_but", layout.quit_but);
        confprint("cam_ret", layout.cam_ret);
}

void js_config_mode(FILE *config, int fd)
{
        struct js_layout layout;

        /*
         * Returns a funciton that waits for either a button or an axis
         * To be pressed, then returns what button/axis was pressed
         */
        auto input_builder = [fd](int type)
                {
                        return [fd, type](const char *msg)
                        {

                                struct js_event js;
                                fprintf(stderr, "Press the %s.\n\n", msg);
                                /* wait for button or axis */
                                errno = 0;

                                /* eat events */
                                while(errno != EAGAIN)
                                        read(fd, &js, sizeof(js));
                                js = {0, 0, 0, 0};
                                while (read(fd, &js, sizeof(js)),
                                       js.type != type || errno != EAGAIN)  {
                                        errno = 0;
                                }

                                fprintf(stderr,
                                        "Read %d, please press enter \n",
                                        js.number);

                                getchar();

                                return js.number;
                        };
                };

        /* funcion to wait for axis input */
        auto get_ax = input_builder(JS_EVENT_AXIS);
        /* function to wait for button input */
        auto get_but = input_builder(JS_EVENT_BUTTON);

        /* Take input for buttons and axes */
        layout.x_ax = get_ax("Strafe axis");
        layout.y_ax = get_ax("Vertical axis");
        layout.z_ax = get_ax("Forward reverse axis");
        layout.rot_ax = get_ax("Rotational axis");
        layout.quit_but = get_but("Quit button");
        layout.cam_ret = get_but("Camera recenter button");

        js_write_config(config, layout);
        close(fd);
}

int main(int argc, char **argv)
{
        int jsfd;
        FILE *config;
        if (argc < 3) {
                fprintf(stderr, "Usage: %s JSPATH CONFPATH\n", argv[0]);
                return 0;
        }

        if (jsfd = open(argv[1], O_RDONLY | O_NONBLOCK), jsfd < 0) {
                perror("open");
                return -1;
        }

        if (config = fopen(argv[2], "w"), !config) {
                perror("fopen");
                return -1;
        }

        js_config_mode(config, jsfd);
        close(jsfd);
        fclose(config);
        return 0;
}
