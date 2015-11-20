#pragma once
#include <stdint.h>
#include <linux/joystick.h>
#include <array>
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
                void update(js_event &event);
                std::array<int, 6> to_motors(const js_layout &layout);
                void to_motors(const js_layout &layout, std::array<int, 6> &motors);
                ~js_log();
        };

        struct motor_vals {
                int A, B, C, D, V;
        };


        /* void send_motors(int fd, std::array<int, 6> &motors, int opt); */

        void send_motors(int fd, std::array<int, 6> &motors, int opt = 0);
/* void send_motors(FILE *out, struct jsmath::motor_vals &motors); */
}
