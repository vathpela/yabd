// SPDX-License-Identifier: GPLv3-or-later
/*
 * bindiff.c - diff some bin
 * Copyright Peter Jones <pjones@redhat.com>
 */

#include "bindiff.h"

#include <getopt.h>
#include <xdiff/xinclude.h>

int verbose = 1;

static void NORETURN
usage(int ret)
{
	FILE *out = ret == 0 ? stdout : stderr;
	fprintf(out,
	        "Usage: %s [OPTION...] FILE FILE\n"
	        "Help options:\n"
	        "  -?, --help                        Show this help message\n"
	        "      --usage                       Display brief usage message\n",
	        program_invocation_short_name);
	exit(ret);
}

int
main(int argc, char *argv[])
{
	char *sopts = "v?";
	struct option lopts[] = { { "help", no_argument, 0, '?' },
		                  { "quiet", no_argument, 0, 'q' },
		                  { "unified", no_argument, 0, 'u' },
		                  { "usage", no_argument, 0, 0 },
		                  { "verbose", no_argument, 0, 'v' },
		                  { 0, 0, 0, 0 } };
	int c = 0;
	int i = 0;
	char *files[] = { NULL, NULL };

	while ((c = getopt_long(argc, argv, sopts, lopts, &i)) != -1) {
		printf("c:%d optarg:\"%s\"\n", c, optarg);
		switch (c) {
		case 'q':
			verbose -= 1;
			if (verbose < 0)
				verbose = 0;
			break;
		case 'u':
			/* for compatibility */
			break;
		case 'v':
			verbose += 1;
			if (verbose < 0 || verbose > INT_MAX)
				verbose = INT_MAX;
			break;
		case '?':
			usage(EXIT_SUCCESS);
			break;
		case 0:
			if (strcmp(lopts[i].name, "usage"))
				usage(EXIT_SUCCESS);
			break;
		}
	}

	for (; optind < argc; optind++) {
		int x;
		for (x = 0; x < 2; x++) {
			if (files[x] == NULL) {
				files[x] = argv[optind];
				break;
			}
		}
		if (x == 2) {
			warnx("too many arguments");
			usage(EXIT_FAILURE);
		}
	}
	if (!files[0] || !files[1]) {
		warnx("too few arguments");
		usage(EXIT_FAILURE);
	}

	return 0;
}

// vim:fenc=utf-8:tw=75:noet
