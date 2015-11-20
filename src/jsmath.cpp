#include <sys/ioctl.h>          /* For ioctl() */
#include <math.h>               /* For hypot(), fabs(), and degrees() */
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "../include/bstrwrap.h" /* For CBString */
#include "../include/jsmath.hpp" /* For function and struct definitions */
#include "../include/exceptions.hpp"

#ifndef M_PI
#define M_PI 3.14159265
#endif
namespace jsmath {


        const int horizontal_max = 1000;
        const int horizontal_mid = horizontal_max / 2;
        const int horizontal_offset = 1000;
        const int jsmax = 32767;

        /* OTHER FUNCTIONS */
        /*
        * Quadrent based on x and y coordinates. y is flipped because the joystick reads a
        * negative value at the top. Equivalent to the quadrents of the unit circle.
        */
        int quadrent(int x, int y)
        {
                if (x >= 0 && y <= 0)
                        return 1;
                else if (x < 0 && y <= 0)
                        return 2;
                else if (x < 0 && y > 0)
                        return 3;
                else if (x >= 0 && y > 0)
                        return 4;
                return 0;
        }

        /* Converts an integer range i from [min, max] to [omin,omax] */
        double map_v(double i, double min, double max, double omin, double omax)
        {
                double rtn = (i - min) * (omax - omin) / (max - min) + omin;
                return rtn;
        }

        /* Converts radians to degrees */
        double degrees(double rad)
        {
                return rad * 360 / (2 * M_PI);
        }

        /* Gives the radius of a circle based on x and y coordinates */
        double r(double x, double y)
        {
                return hypot(x, y)/jsmax;
        }

        /* Gives the angle of a coordinate in a circle */
        double t(double x, double y)
        {
                if (!x)
                        return 90;
                return fabs(degrees(atan(y / x)));
        }


        /* Deallocate memory for inputs */
        void free_input(struct jsmath::js_log &map)
        {
                delete[] map.ax;
                delete[] map.but;
                map.ax = 0;
                map.but = 0;
        }

        /* Get strafe values for the motors */
        void get_strafe(int x, int y, int &AC, int &BD)
        {
                double r1, t1;

                r1 = r(x, y);
                t1 = t(x, y);

                /* make functions that automatically use r1 and t1 with proper values */
                auto rmap = [=](double min, double max)
                        {
                                return map_v(r1, 0., 1., min, max);
                        };
                auto tmap = [=](double min, double max)
                        {
                                return map_v(t1, 0., 90., min, max);
                        };

                switch(quadrent(x, y)) {
                case 1:
                        /* equavlent to map_v(r1, 0, 1, horizontal_mid, horizontal_max) */
                        AC = rmap(horizontal_mid, horizontal_max);
                        BD = tmap(horizontal_max - AC, AC); /* right */
                        break;
                case 2:
                        BD = rmap(horizontal_mid, horizontal_max);
                        AC = tmap(horizontal_max - BD, BD);
                        break;
                case 3:
                        AC = rmap(horizontal_mid, 0);
                        BD = tmap(horizontal_max - AC, AC);
                        break;
                case 4:
                        BD = rmap(horizontal_mid, 0);
                        AC = tmap(horizontal_max - BD, BD);
                        break;
                case 0:
                        throw_js_exception("Joystick outside of any quadrent");
                }
        }

        /* Makes sure the motor value isn't outside of any bounds and adds the offset */
        int fix_motor(int val)
        {
                if (val > horizontal_max)
                        return horizontal_max + horizontal_offset;
                else if (val < 0)
                        return 0 + horizontal_offset;
                return val + horizontal_offset;
        }

        void send_motors(int fd, std::array<int, 6> &motors, int opt)
        {
                Bstrlib::CBString buf;
                int i = 0;
                for (auto &x : motors)
                        buf.formata("%d=%d\n", i++, x);
                if (write(fd, (const char *)buf, buf.length()) < 0)
                        throw_sys_exception("send(%d, %s)", fd, (const char *)buf);
        }


        js_log::js_log(int fd) : fd(fd) {
                numax = js_num_ax(fd);
                numbut = js_num_but(fd);
                ax = new int[numax];
                but = new int[numbut];

                if (!ax || !but)
                        throw_sys_exception("new int[]");

                /* Zero array */
                memset(ax, 0, sizeof(int) * numax);
                memset(but, 0, sizeof(int) * numbut);

        }

        js_log::~js_log() {
                if (ax && but && numax && numbut) {
                        delete[] ax;
                        delete[] but;
                }

        }

        /* update log with new event */
        void js_log::update(js_event &event)
        {
                if (!ax || !but)
                        throw_js_exception("Unallocated memory for button inputs");
                /* Remove initial differentiation between initial events */
                event.type &= ~JS_EVENT_INIT;
                if (event.type == JS_EVENT_AXIS) {
                        if (event.number >= numax)
                                throw_js_exception("Joystick axis event out of bounds. (%d max %d)", event.number, numax);
                        ax[event.number] = event.value;
                } else if (event.type == JS_EVENT_BUTTON) {
                        if (event.number >= numbut)
                                throw_js_exception("Joystick button event out of bounds. (%d max %d)", event.number, numbut);
                        but[event.number] = event.value;
                }
                last = &event;
        }

        std::array<int, 6> js_log::to_motors(const js_layout &layout)
        {
                std::array<int, 6> motors;

                double rot;
                int AC, BD;

                motors[4] = map_v(ax[layout.y_ax], jsmax, -jsmax, 1100, 1800);
                motors[5] = motors[4];
                rot = map_v(ax[layout.rot_ax], jsmax, -jsmax, -horizontal_mid, horizontal_mid);

                get_strafe(ax[layout.z_ax], ax[layout.x_ax], AC, BD);

                motors[0] = fix_motor(horizontal_max - AC + rot);
                motors[1] = fix_motor(BD - rot);
                motors[2] = fix_motor(horizontal_max - AC - rot);
                motors[3] = fix_motor(BD + rot);
                return motors;
        }

        void js_log::to_motors(const js_layout &layout, std::array<int, 6> &motors)
        {
                double rot;
                int AC, BD;

                motors[4] = map_v(ax[layout.y_ax], jsmax, -jsmax, 1100, 1800);
                motors[5] = motors[4];
                rot = map_v(ax[layout.rot_ax], jsmax, -jsmax, -horizontal_mid, horizontal_mid);

                get_strafe(ax[layout.z_ax], ax[layout.x_ax], AC, BD);

                motors[0] = fix_motor(horizontal_max - AC + rot);
                motors[1] = fix_motor(BD - rot);
                motors[2] = fix_motor(horizontal_max - AC - rot);
                motors[3] = fix_motor(BD + rot);
        }


}
