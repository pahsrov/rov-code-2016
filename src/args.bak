#include <stdio.h>
#include <stdlib.h>
#include "../include/args.hpp"
#include "../include/exceptions.hpp"

void print_help(const char *prog_name)
{
				fprintf(stderr,
                "Usage: %s [options] joy_path port ipaddr\n\n"
                "Options:\n"
                "\t-h\t\tPrint this help message and exit\n"
                "\t-s\t\tWrite to stdout instead of a socket\n"
                "\t-C\t\tEnter config mode\n"
                "\t-c conf_path\tUse a different config path than usual (default: joystick.conf)\n"
                , prog_name);
}

void print_usage(const char *prog_name)
{
        fprintf(stderr, "Usage: %s [options] joy_path port ipaddr\n", prog_name);
}

void handle_flags(int &argc, char **argv, struct rov_opts &opt)
{
				int i, j;

				/* for every command line argument */
				for (i = 1; i < argc; ++i) {
								/* options start with - */
								if (argv[i][0] == '-') {
												switch(argv[i][1]) {
												case 'h': /* print help */
																print_help(argv[0]);
																exit(0);
												case 's': /* print to stdout instead of socket */
																opt.use_stdout = 1;
																break;
												case 'C': /* conf mode */
																opt.conf_mode = 1;
																break;
												case 'c': /* change config path */

																/* if there's nothing in the next spot */
																if (argc < i + 2)
																				throw_js_exception("No config path specified after -c flag");

																/* set the spot after we found the flag to the new path */
																opt.conf_path = argv[i + 1];
																break;
												default:
																throw_js_exception("Unable to parse flag %c. Did you mean to type -h for help?\n", argv[i][1]);
												}

												/* remove argv[i + 1] from argv */
												--argc;
												for (j = i; j < argc; j++)
																argv[j] = argv[j + 1];

												/* deincrement i so that it doesn't skip the next argument */
												--i;
								}
				}
}
