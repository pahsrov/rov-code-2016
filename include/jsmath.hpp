#pragma once
#include <linux/joystick.h>
#include <array>
#include <vector>
#include "js.hpp"

class js_log {
private:
        js_log();
        /* axes */
        std::vector<int> ax;

        /* buttons */
        std::vector<int> but;
public:

        js_log(int fd);
        void update(const js_event &event);
        void to_motors(const js_layout &layout, std::array<int, 6> &motors);
        std::array<int, 6> to_motors(const js_layout &layout);
        int numax();
        int numbut();
};

struct motor_vals {
        int A, B, C, D, V;
};


void send_motors(int fd, std::array<int, 6> &motors);
