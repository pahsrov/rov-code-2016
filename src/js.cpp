#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <termios.h>
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

/* reads an event from the joystick */
int js_read(int fd, struct js_event &js)
{
        errno = 0;
        /* read from joystick */
        if (read(fd, &js, sizeof(struct js_event)) < 0) {
                /* there was nothing to be read */
                if (errno == EAGAIN)
                        return 1;

                /* otherwise, something went wrong */
                throw_sys_exception("read");
        }

        /*
         * Linux differentiates between the first inputs sent by the joystick
         * after opening all the others. This removes this behaviour.
         */
        js.type &= ~JS_EVENT_INIT;
        return 0;
}

int js_quit(const struct js_event &js, const struct js_layout &layout)
{
        /* Detects if the quit button in the layout has been pressed. */
        if (js.value == 1 && js.type == JS_EVENT_BUTTON &&
            js.number == layout.quit_but)
                return 1;
        return 0;
}

/* matches a string with a config variable */
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

void js_load_config(FILE *config, struct js_layout &layout)
{
        char *var;
        int val;
        int line_num = 0;       /* for error recording */
        int err;
        /* zero layout */
        memset(&layout, 0, sizeof(layout));

        /* take input until something goes wrong */
        while (err = fscanf(config, "%ms = %d", &var, &val), err > 0) {
                line_num++;

                /* match input to a variable in layout then set that variable*/
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
                        throw_js_exception("Could not parse line %d in config.\n"
                                           "%s = %d", line_num, var, val);
                }
                free(var);
        }

        if (err == 2)
                throw_js_exception("fscanf matching error");

        /* either the file ended or something went wrong */
        if (ferror(config))
                throw_sys_exception("fscanf");
        else if (!feof(config))
                 throw_js_exception("Unable to parse line %d in config file",
                                    line_num);

}

/* returns number of axes on joystick */
int js_num_ax(int fd)
{
        int ax;
        if (ioctl(fd, JSIOCGAXES, &ax) < 0)
                throw_sys_exception("ioctl: fd = %d", fd);
        return ax;
}

/* return number of buttons on joystick */
int js_num_but(int fd)
{
        int but;
        if (ioctl(fd, JSIOCGBUTTONS, &but) < 0)
                throw_sys_exception("ioctl: fd = %d", fd);
        return but;
}
