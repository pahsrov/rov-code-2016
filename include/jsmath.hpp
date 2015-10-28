#pragma once
#include <stdint.h>
#include <linux/joystick.h>
#include "js.hpp"

namespace jsmath {

        struct js_map {
                /* axes */
                int *ax;

                /* buttons */
                int *but;

                /* last event read */
                struct js_event *last;

                int numax, numbut;
                int allocd;

                int fd;

                js_map(int fd);
                js_map();
                ~js_map();
        };

        struct js_send {
                int A, B, C, D, V;
        };

        void read_to_map(struct jsmath::js_map &map, struct js_event &event);

        void get_num_input(struct jsmath::js_map &map, int fd);

        void allocate_input(struct jsmath::js_map &map);

        void free_input(struct jsmath::js_map &map);

        int map_to_send(struct jsmath::js_send &motors,
                        const struct jsmath::js_map &map, const struct js_layout &layout);

        int sender(int fd, struct jsmath::js_send motors, long int (*sender)(int fd, const void *, size_t));
}
