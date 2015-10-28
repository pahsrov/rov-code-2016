#pragma once
#include <stdarg.h>
#include <errno.h>
#include "bstrwrap.h"

#define throw_sys_exception(MSG, ...) do {throw sys_exception(__LINE__, __func__, __FILE__, MSG, ##__VA_ARGS__);} while(0)
#define throw_js_exception(MSG, ...) do {throw js_exception(__LINE__, __func__, __FILE__, MSG, ##__VA_ARGS__);} while(0)

/* Don't ask me what this does */
struct sys_exception : public std::exception {
private:
        Bstrlib::CBString m_msg;
        int err;
public:
        sys_exception(int line, const char *func, const char *file, const char *msg, ...)
                {
                        m_msg.format("ERROR: ");
                        bvformata(err, (bstring) &m_msg, msg, msg);
                        m_msg.formata("\nLINE: %d\n"
                                      "Function: %s\n"
                                      "File: %s\n", line, func, file);
                        err = errno;
                        m_msg += "\nstrerror: ";
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
        js_exception(int line, const char *func, const char *file, const char *msg, ...)
                {
                        m_msg.format("ERROR: ");
                        bvformata(err, (bstring) &m_msg, msg, msg);
                        m_msg.formata("\nLINE: %d\n"
                                      "Function: %s\n"
                                      "File: %s\n", line, func, file);
                }
        virtual ~js_exception () throw () {}
        virtual const char *what () const throw () {return (const char *)m_msg;}
};
