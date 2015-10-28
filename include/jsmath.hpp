#pragma once
#include <stdint.h>
#include <linux/joystick.h>
#include <vector>
#include "js.hpp"

namespace jsmath {

        struct js_log {
                /* axes */
                int *ax;

                /* buttons */
                int *but;

                /* last event read */
                struct js_event *last;

                int numax, numbut;
                int allocd;

                int fd;

                js_log(int fd);
                js_log();
                ~js_log();
        };

        struct motor_vals {
                int A, B, C, D, V;
        };

        void event_to_log(struct js_event &event, struct jsmath::js_log &map);

        void allocate_input(struct jsmath::js_log &map);

        void free_input(struct jsmath::js_log &map);

        void log_to_motors(struct jsmath::motor_vals &motors,
                        const struct jsmath::js_log &map, const struct js_layout &layout);

        void send_motors(int fd, struct jsmath::motor_vals &motors, int opt);

void send_motors(FILE *out, struct jsmath::motor_vals &motors);
}
