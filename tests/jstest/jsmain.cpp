#include <stdio.h>
#include <fcntl.h>
#include "../include/js.hpp"
#include "../include/exceptions.hpp"

int main(int argc, char **argv)
{
        int jsfd;
        struct js_event js;

        if (argc < 2)
                return -1;

        jsfd = open(argv[1], O_RDONLY);

        for(;;)  {
                try {
                        js_read(jsfd, js);
                } catch (sys_exception e) {
                        fprintf(stderr, "%s\n", e.what());
                        return -1;
                }
                printf("%d, %d, %d, %d\n", js.time, js.value, js.type, js.number);
        }

        return 0;
}
