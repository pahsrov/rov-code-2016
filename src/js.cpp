#include <unistd.h>
#include <cerrno>
#include "../include/js.hpp"
#include "../include/bstrwrap.h"
#include "../include/exceptions.hpp"

enum class conf_var_name {
        nomatch,       //0
        xaxis,         //1
        yaxis,         //2
        zaxis,         //etc
        rotaxis,
        qbut,
        camret
};


int js_read(int fd, struct js_event &js)
{
        errno = 0;
        /* read from joystick */
        if (read(fd, &js, sizeof(struct js_event)) < 0) {
                /* there was nothing to be read */
                if (errno == EAGAIN)
                        return 1;

                /* otherwise, something went wrong */
                throw sys_exception("read");
        }

        /*
         * Linux differentiates between the first inputs sent by the joystick after opening
         * all the others. This removes this behaviour.
         */
        return 0;
}

int js_quit(const struct js_event &js, const struct js_layout &layout)
{
        /* Detects if the quit button in the layout has been pressed. */
        if (js.value == 1 && js.type == JS_EVENT_BUTTON && js.number == layout.quit_but)
                return 1;
        return 0;
}

enum conf_var_name js_match_confvar(const char *var)
{
        /* Returns an integer (enum) based on what variable we have. */
        CBString str = var;
        if (str == "x_ax")
                return conf_var_name::xaxis;
        if (str == "y_ax")
                return conf_var_name::yaxis;
        if (str == "z_ax")
                return conf_var_name::zaxis;
        if (str == "rot_ax")
                return conf_var_name::rotaxis;
        if (str == "quit_but")
                return conf_var_name::qbut;
        if (str == "cam_ret")
                return conf_var_name::camret;
        return conf_var_name::nomatch;
}

void js_write_config(FILE *config, const struct js_layout &layout)
{
        /* function to print to the config file */
        auto confprint = [=](const char *var, int val)
                {
                        if (fprintf(config, "%s = %d\n", var, val) < 0)
                                throw sys_exception("fprintf");
                };

        confprint("x_ax", layout.x_ax);
        confprint("y_ax", layout.y_ax);
        confprint("z_ax", layout.z_ax);
        confprint("rot_ax", layout.rot_ax);
        confprint("quit_but", layout.quit_but);
        confprint("cam_ret", layout.cam_ret);
}

void js_write_def_config(FILE *config)
{
        /* set an initial layout */
        struct js_layout layout = (struct js_layout) {
                .x_ax = 0,
                .y_ax = 1,
                .z_ax = 2,
                .rot_ax = 3,
                .quit_but = 4,
                .cam_ret = 5
        };

        /* Apply it */
        js_write_config(config, layout);

}

void js_config_mode(FILE *config, int fd)
{
        struct js_event js;
        struct js_layout layout;

        /*
         * Returns a funciton that waits for either a button or an axis
         * To be pressed, then returns what button/axis was pressed
         */
        auto input_builder = [&js, fd](int type)
                {
                        return [&js, fd, type](const char *msg)
                        {
                                fprintf(stderr, "Press the %s, then press start.\n\n", msg);
                                int err;
                                /* wait for button or axis */
                                do {
                                        err = read(fd, &js, sizeof(struct js_event));
                                } while (js.type != type && err > 0);

                                if (errno != EAGAIN)
                                        throw sys_exception("read");
                                /* Wait for user to press enter */
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
}

void js_load_config(FILE *config, struct js_layout &layout)
{
        char *var;
        int val;
        int line_num = 0;       /* for error recording */

        /* take input until something goes wrong */
        while (fscanf(config, "%ms = %d", &var, &val) == 2) {
                line_num++;

                /* match input to a variable in layout */
                switch (js_match_confvar(var)) {
                case conf_var_name::xaxis:
                        layout.x_ax = val;
                        break;
                case conf_var_name::yaxis:
                        layout.y_ax = val;
                        break;
                case conf_var_name::zaxis:
                        layout.z_ax = val;
                        break;
                case conf_var_name::rotaxis:
                        layout.rot_ax = val;
                        break;
                case conf_var_name::qbut:
                        layout.quit_but = val;
                        break;
                case conf_var_name::camret:
                        layout.cam_ret = val;
                        break;
                default:
                        throw js_exception("Could not parse line %d in config.\n"
                                           "%s = %d", line_num, var, val);
                }
        }

        /* either the file ended or something went wrong */
        if (ferror(config))
                throw sys_exception("fscanf");
         else if (!feof(config))
                 throw js_exception("Unable to parse line %d in config file", line_num);
}
