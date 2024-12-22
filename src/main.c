#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#include "tree.h"
#include "html.h"

int main(int argc, char *argv[]) {
	bool invalid = false;
	int opt;
	struct options opts = {0};
	opts.dotfiles = true;

	// argument handling
	while ((opt = getopt_long(argc, argv, ":hVmdr", (struct option[]){
	                                                        {"help",        no_argument, 0, 'h'},
	                                                        {"version",     no_argument, 0, 'V'},
	                                                        {"metric",      no_argument, 0, 'm'},
	                                                        {"no-dotfiles", no_argument, 0, 'A'},
	                                                        {"reverse",     no_argument, 0, 'r'},
	                                                        {0,             0,           0, 0  }
    },
	                          NULL)) != -1) {
		switch (opt) {
			case 'h':
				printf("Usage: %s [OPTION]... [FILE]...\n", PROJECT_NAME);
				printf("-h --help: Shows help text\n");
				printf("-V --version: Shows the version\n");
				printf("-m --metric: Use powers of 1000 instead of 1024 for file sizes\n");
				printf("-A --no-dotfiles: Ignores files starting with a dot\n");
				printf("-r --reverse: Items are ordered from small to big instead of big to small\n");
				return 0;
			case 'V':
				printf("%s %s\n", PROJECT_NAME, PROJECT_VERSION);
#ifdef PROJECT_URL
				printf("See %s\n", PROJECT_URL);
#endif
				return 0;
			default:
				switch (opt) {
					case 'm':
						opts.metric = true;
						break;
					case 'A':
						opts.dotfiles = false;
						break;
					case 'r':
						opts.reverse = true;
						break;
					default:
						invalid = true;
						break;
				}
		}
	}

	if (invalid) {
		// use `argc - 1` if you want to require one non-flag argument, such as a file name
		eprintf("Invalid usage, try --help\n");
		return 1;
	}

	struct file_node *head = NULL, *tail = NULL;

	if (optind >= argc) {
		// include current directory if no arguments are given
		head = tree_build(".", opts);
	} else {
		for (int i = optind; i < argc; i++) {
			struct file_node *node = tree_build(argv[i], opts);
			if (!head) {
				// first node
				head = tail = node;
			} else {
				// append node
				tail->next = node;
				tail = node;
			}
		}
	}

	if (head) {
		// html_print(head);
		tree_free(head);
	} else {
		eprintf("No files processed\n");
		return 1;
	}

	return 0;
}
