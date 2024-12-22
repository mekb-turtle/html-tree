#include <sys/stat.h>
#include <stdbool.h>

struct file_node {
	char *name;
	char *path;
	size_t size;
	size_t num_items;
	mode_t mode;
	struct file_node *children;
	struct file_node *next;
};

struct options {
	bool metric, dotfiles, reverse;
};

struct file_node *tree_build(const char *path, struct options opts);
void tree_free(struct file_node *node);
