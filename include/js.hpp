#pragma once
#include <linux/joystick.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bstrwrap.h"

struct js_layout {
        /* x is strafe, y is vertical, z is forward/reverse */
        int x_ax, y_ax, z_ax, rot_ax;

        /* add more commands as more rov control is needed (ex claw)*/
        int quit_but;           /* exit program button */
        int cam_ret;            /* return servos to default position */
        int claw_but;
};


int js_read(int jsfd, struct js_event &js);

int js_quit(const struct js_event &js, const struct js_layout &layout);

int js_num_ax(int fd);

int js_num_but(int fd);

void js_load_config(FILE *config, struct js_layout &layout);

void js_write_config(FILE *config, const struct js_layout &layout);

void js_write_default_config(FILE *config);

void js_config_mode(FILE *config, const char *jspath);
