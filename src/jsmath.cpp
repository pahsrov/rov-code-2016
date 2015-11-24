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

/* proper values for seabotix goes from 1000-2000 (so 0-1000 + 1000) */
const int horizontal_max = 1000;
const int horizontal_mid = horizontal_max / 2;
const int horizontal_offset = 1000;
const int jsmax = 32767;

/* OTHER FUNCTIONS */
/*
 * Quadrent based on x and y coordinates.
 * y is flipped because the joystick reads a negative value at the top.
 * Equivalent to the quadrents of the unit circle.
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
inline double map_v(double i, double min, double max, double omin, double omax)
{
        return (i - min) * (omax - omin) / (max - min) + omin;
}

/* Converts radians to degrees */
inline double degrees(double rad)
{
        return rad * 360 / (2 * M_PI);
}

/* Gives the radius of a circle based on x and y coordinates */
inline double r(double x, double y)
{
        return hypot(x, y)/jsmax;
}

/* Gives the angle of a coordinate in a circle */
inline double t(double x, double y)
{
        if (!x)
                return 90;
        return fabs(degrees(atan(y / x)));
}

/*
 * Take the joystick's two axis values (vertical and horizontal)
 * as x and y values of a line in the unit circle (see drawing).
 * Then get the radius of the circle and the angle of the point.
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

        /* make functions that use r1 and t1 with proper values */
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
                /* same as map_v(r1, 0, 1, horizontal_mid, horizontal_max) */
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

/* Makes sure the motor value isn't out of bounds and adds offset */
int fix_motor(int val)
{
        if (val > horizontal_max)
                return horizontal_max + horizontal_offset;
        else if (val < 0)
                return 0 + horizontal_offset;
        return val + horizontal_offset;
}

/* sends the motor values to the arudino */
void send_motors(int fd, std::array<int, 6> &motors)
{
        /* make a string to send down */
        Bstrlib::CBString buf;
        int i = 0;

        /*
         * Go over each motor value and append which motor to send and
         * its value to the string to send.
         * ex. 1=2000 means set motor 1 to 2000 (full forward)
         */
        for (auto &x : motors)
                buf.formata("%d=%d\n", i++, x);

        /* send the string to the arduino */
        if (write(fd, (const char *)buf, buf.length()) < 0)
                throw_sys_exception("send(%d, %s)",
                                    fd, (const char *)buf);
}

/* Make vectors for buttons and axes and initialize them to zero*/
js_log::js_log(int fd) : ax(js_num_ax(fd), 0), but(js_num_but(fd), 0)
{
}


/* update log with new event */
void js_log::update(const js_event &event)
{
        /*
         * put the value of the event (ex. 1 or 0 for a button)
         * into the array of button or axis values
         *
         * For example: assuming start is button zero,
         * pressing start would set 1 into but[0]
         */
        if (event.type == JS_EVENT_AXIS) /* axis event */
                ax[event.number] = event.value;
        else if (event.type == JS_EVENT_BUTTON) /* button event */
                but[event.number] = event.value;
}


/* Converts from the log to the motor values to send to the arduino */
void js_log::to_motors(const js_layout &layout, std::array<int, 6> &motors)
{
        double rot;
        int AC, BD;

        /* motors 4 and 5 are the 2 vertical motors */
        motors[4] = map_v(ax[layout.y_ax], jsmax, -jsmax, 1100, 1800);
        motors[5] = motors[4];

        /* rotation to add to horizontal motors */
        rot = map_v(ax[layout.rot_ax], jsmax, -jsmax,
                    -horizontal_mid, horizontal_mid);

        /* strafe to add to motors */
        get_strafe(ax[layout.z_ax], ax[layout.x_ax], AC, BD);

        /* add strafe and rotation to each horizontal motor, then add offset */
        motors[0] = fix_motor(horizontal_max - AC + rot);
        motors[1] = fix_motor(BD - rot);
        motors[2] = fix_motor(horizontal_max - AC - rot);
        motors[3] = fix_motor(BD + rot);
}

std::array<int, 6> js_log::to_motors(const js_layout &layout)
{
        std::array<int, 6> motors;
        double rot;
        int AC, BD;

        /* motors 4 and 5 are the 2 vertical motors */
        motors[4] = map_v(ax[layout.y_ax], jsmax, -jsmax, 1100, 1800);
        motors[5] = motors[4];

        /* rotation to add to horizontal motors */
        rot = map_v(ax[layout.rot_ax], jsmax, -jsmax,
                    -horizontal_mid, horizontal_mid);

        /* strafe to add to motors */
        get_strafe(ax[layout.z_ax], ax[layout.x_ax], AC, BD);

        /* add strafe and rotation to each horizontal motor, then add offset */
        motors[0] = fix_motor(horizontal_max - AC + rot);
        motors[1] = fix_motor(BD - rot);
        motors[2] = fix_motor(horizontal_max - AC - rot);
        motors[3] = fix_motor(BD + rot);
        return motors;
}
