#pragma once
#include <linux/joystick.h>
#include <stdio.h>


struct js_layout {
        /* x is strafe, y is vertical, z is forward/reverse */
        int x_ax, y_ax, z_ax, rot_ax;

        /* add more commands as more rov control is needed (ex claw)*/
        int quit_but;           /* button to exit code */
        int cam_ret;            /* button to return servos to default position */
};

int js_read(int jsfd, struct js_event *js);

int js_quit(const struct js_event *restrict js, const struct js_layout *restrict layout);

int js_load_config(FILE *config, struct js_layout *layout);

int js_write_config(FILE *config, const struct js_layout *layout);

int js_write_def_config(FILE *config);

void js_config_mode(FILE *config, int fd);
