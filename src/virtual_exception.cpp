#include <stdio.h>
#include <string>

struct my_exception : public std::exception {
        my_exception()
                {
                }
        virtual ~my_exception () throw () {}
        virtual const char *what () const throw () {return "my_exception";} 
};

int main()
{
        try {
                throw my_exception();
        } catch (std::exception &e) {
                fprintf(stderr, "ERROR: %s\n", e.what());
        }
        return 0;
}
