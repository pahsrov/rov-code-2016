#pragma once
#include <stdarg.h>
#include <errno.h>
#include "bstrwrap.h"

/* Don't ask me what this does */
struct sys_exception : public std::exception {
private:
        Bstrlib::CBString m_msg;
        int err;
public:
        sys_exception(const char *msg, ...)
                {
                        bvformata(err, (bstring) &m_msg, msg, msg);
                        err = errno;
                        m_msg += "\nERROR: ";
                        m_msg += strerror(err);
                }
        virtual ~sys_exception () throw () {}
        virtual const char *what () const throw () {return (const char *)m_msg;}
};

struct js_exception : public std::exception {
private:
        Bstrlib::CBString m_msg;
        int err;
public:
        js_exception(const char *msg, ...)
                {
                        bvformata(err, (bstring) &m_msg, msg, msg);
                }
        virtual ~js_exception () throw () {}
        virtual const char *what () const throw () {return (const char *)m_msg;}
};
