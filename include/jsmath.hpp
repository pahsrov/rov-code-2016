#pragma once
#include <linux/joystick.h>
#include <array>
#include <vector>
#include "js.hpp"

class js_log {
private:
        js_log();
public:
        /* axes */
        std::vector<int> ax;

        /* buttons */
        std::vector<int> but;

        js_log(int fd);
        void update(js_event &event);
        void to_motors(const js_layout &layout, std::array<int, 6> &motors);
        int numax();
        int numbut();
};

struct motor_vals {
        int A, B, C, D, V;
};


void send_motors(int fd, std::array<int, 6> &motors);
