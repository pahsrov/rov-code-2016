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


        /* proper values for seabotix goes from 1000-2000 (so 0-1000 + 1000) */
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

        /*
         * Converts an integer range i from [min, max] to [omin,omax]
         * For example, map_v(i, 0, 100, 0, 1000) would convert a range
         * of 0-100 to 0-1000 (so 50 in i would return 500)
         */
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


        /*
         * Take the joystick's two axis values (vertical and horizontal)
         * as x and y values of a line in the unit circle (see drawing). Thenn get the radius of the
         * circle and the angle of the point.
         *                            -----+-----
         *                        ---/     |     \---
         *                     --/         |        /\--
         *                    /            |      /-    \
         *                   /             |  r /-       \
         *                  /              |  /-..        \
         *                 /               |/-   .. t      \
         *                 +---------------/--------------+
         *                 \               |               /
         *                  \              |              /
         *                   \             |             /
         *                    \            |            /
         *                     --\         |         /--
         *                        ---\     |     /---
         *                            -----+-----
         */
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

        /* sends the motor values to the arudino */
        void send_motors(int fd, std::array<int, 6> &motors, int opt)
        {
                /* make a string to send down */
                Bstrlib::CBString buf;
                int i = 0;

                /*
                 * Go over each motor value and append which motor to send and its value
                 * to the string to send.
                 *
                 * ex. 1=2000 means set motor 1 to 2000 (full forward)
                 */
                for (auto &x : motors)
                        buf.formata("%d=%d\n", i++, x);

                /* send the string to the arduino */
                if (write(fd, (const char *)buf, buf.length()) < 0)
                        throw_sys_exception("send(%d, %s)", fd, (const char *)buf);
        }


        js_log::js_log(int fd) : fd(fd) {
                numax = js_num_ax(fd);
                numbut = js_num_but(fd);

                /* allocate arrays based on number of axes and buttons */
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
                /* if the axis or button arrays don't exist */
                if (!ax || !but)
                        throw_js_exception("Unallocated memory for button inputs");
                /* Remove initial differentiation between initial events */
                event.type &= ~JS_EVENT_INIT;

                if (event.type == JS_EVENT_AXIS) { /* axis event */
                        if (event.number >= numax)
                                throw_js_exception("Joystick axis event out of bounds. (%d max %d)", event.number, numax);
                        /*
                         * put the event value into the array of axis values
                         * ex. ax[4]     = 32767 means that axis 4 is full forward
                         */
                        ax[event.number] = event.value;
                } else if (event.type == JS_EVENT_BUTTON) { /* button event */
                        if (event.number >= numbut)
                                throw_js_exception("Joystick button event out of bounds. (%d max %d)", event.number, numbut);
                        /*
                         * put the event value into the array of button values
                         * ex. but[4] = 1 means that button 4 has been pressed
                         */
                        but[event.number] = event.value;
                }

                /* copy the last event to the log */
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

        /* Converts from the log file to the final motor values to send to the arduino */
        void js_log::to_motors(const js_layout &layout, std::array<int, 6> &motors)
        {
                double rot;
                int AC, BD;

                /* motors 4 and 5 are the 2 vertical motors */
                motors[4] = map_v(ax[layout.y_ax], jsmax, -jsmax, 1100, 1800);
                motors[5] = motors[4];

                /* rotation to add to horizontal motors */
                rot = map_v(ax[layout.rot_ax], jsmax, -jsmax, -horizontal_mid, horizontal_mid);

                /* strafe to add to motors */
                get_strafe(ax[layout.z_ax], ax[layout.x_ax], AC, BD);

                /* add strafe and rotation to each motor, then add offset */
                motors[0] = fix_motor(horizontal_max - AC + rot);
                motors[1] = fix_motor(BD - rot);
                motors[2] = fix_motor(horizontal_max - AC - rot);
                motors[3] = fix_motor(BD + rot);
        }


}
