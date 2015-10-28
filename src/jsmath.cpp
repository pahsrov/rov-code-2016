#include <sys/ioctl.h>          /* For ioctl() */
#include <math.h>               /* For hypot(), fabs(), and degrees() */
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

        js_map::js_map(int fd) {
                this->fd = fd;
                numax = 0;
                numbut = 0;
                ax = NULL;
                but = NULL;

                get_num_input(*this, fd);
                allocate_input(*this);
        }

        js_map::js_map() {
                numax = 0;
                numbut = 0;
                ax = NULL;
                but = NULL;
        }

        js_map::~js_map() {
                if (ax && but && numax && numbut)
                        free_input(*this);
        }

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
                return (i - min) * (omax - omin) / (max - min) + omin;
        }

        /* Converts radians to degrees */
        double degrees(double rad)
        {
                return rad * 360 / (2 * M_PI);
        }

        /* Gives the radius of a circle based on x and y coordinates */
        double r(int x, int y)
        {
                return hypot(x, y);
        }

        /* Gives the angle of a coordinate in a circle */
        double t(int x, int y)
        {
                if (!y)
                        return 0;
                return fabs(degrees(atan(y / x)));
        }

        /* Updates joystick values with an event */
        void read_to_map(struct jsmath::js_map &js, struct js_event &event)
        {
                if (!js.ax || js.but)
                        throw js_exception("Unallocated memory for button inputs");
                /* Remove initial differentiation between initial events */
                event.type &= ~JS_EVENT_INIT;
                if (event.type == JS_EVENT_AXIS) {
                        if (event.number >= js.numax)
                                throw js_exception("Joystick axis event out of bounds. (%d max %d)", event.number, js.numax);
                        js.ax[event.number] = event.value;
                } else if (event.type == JS_EVENT_BUTTON) {
                        if (event.number >= js.numbut)
                                throw js_exception("Joystick button event out of bounds. (%d max %d)", event.number, js.numbut);
                        js.but[event.number] = event.value;
                }
                js.last = &event;

        }

        /* Get number of buttons and axes */
        void get_num_input(struct jsmath::js_map &map, int fd)
        {
                if (ioctl(fd, JSIOCGAXES, &map.numax) < 0)
                        throw sys_exception("ioctl");
                if (ioctl(fd, JSIOCGBUTTONS, &map.numbut) < 0)
                        throw sys_exception("ioctl");
        }

        /* Allocate memory for storing inputs */
        void allocate_input(struct jsmath::js_map &map)
        {
                map.ax = new int[map.numax];
                map.but = new int[map.numbut];

                if (!map.ax || !map.but)
                        throw sys_exception("new int[]");
                map.allocd = 1;
        }

        /* Deallocate memory for inputs */
        void free_input(struct jsmath::js_map &map)
        {
                delete[] map.ax;
                delete[] map.but;
                map.ax = 0;
                map.but = 0;
        }

        /* Get strafe values for the motors */
        void get_strafe(int x, int y, int &AB, int &CD)
        {
                double r1, t1;

                r1 = r(x, y);
                t1 = t(x, y);

                /* make functions that automatically use r1 and t1 with proper values */
                auto rmap = [=](double min, double max)
                        {
                                return map_v(r1, 0, 1, min, max);
                        };
                auto tmap = [=](double min, double max)
                        {
                                return map_v(t1, 0, 90, min, max);
                        };

                switch(quadrent(x, y)) {
                case 1:
                        /* equavlent to map_v(r1, 0, 1, horizontal_mid, horizontal_max) */
                        AB = rmap(horizontal_mid, horizontal_max);
                        CD = tmap(horizontal_max - AB, AB);
                        break;
                case 2:
                        CD = rmap(horizontal_mid, horizontal_max);
                        AB = tmap(CD, horizontal_max - CD);
                        break;
                case 3:
                        AB = rmap(horizontal_mid, 0);
                        CD = tmap(AB, horizontal_max - AB);
                        break;
                case 4:
                        CD = rmap(horizontal_mid, 0);
                        AB = tmap(horizontal_max - CD, CD);
                        break;
                case 0:
                        throw js_exception("Joystick outside of any quadrent");
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

        /* Uses the layout and the last read motors to get the A B C D and V(ertical) motors */
        int map_to_send(struct jsmath::js_send &motors, const struct jsmath::js_map &map, const struct js_layout &layout)
        {
                const int jsmax = 32727;
                double rot;
                int AB, CD;

                motors.V = map_v(map.ax[layout.y_ax], jsmax, -jsmax, 1100, 1800);
                rot = map_v(map.ax[layout.rot_ax], jsmax, -jsmax, -horizontal_mid, horizontal_mid);

                get_strafe(map.ax[layout.z_ax], map.ax[layout.x_ax], AB, CD);

                motors.A = fix_motor(AB + rot);
                motors.B = fix_motor(AB - rot);
                motors.C = fix_motor(CD + rot);
                motors.D = fix_motor(CD - rot);

                return 0;
        }

        int sender(int fd, struct jsmath::js_send motors, long int (*sender)(int fd, const void *, size_t))
        {
                Bstrlib::CBString buf;

                try {
                        buf.format("A = %d\nB = %d\nC = %d\nD = %d\nV = %d\n",
                                   motors.A, motors.B, motors.C, motors.D, motors.V);
                } catch (Bstrlib::CBStringException e) {
                        fprintf(stderr, "Error: %s\n", e.what());
                        return -1;
                }

                return sender(fd, (const char *)buf, buf.length());
        }

}
