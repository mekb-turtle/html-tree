#ifndef TREE_H
#define TREE_H
#include <sys/stat.h>
#include <stdbool.h>

#include "util.h"

struct file_node {
	char *name;
	char *path;
	size_t size;
	size_t num_items;
	mode_t mode;
	struct file_node *children;
	struct file_node *next;
};

struct file_node *tree_build(const char *path, struct options opts);
void tree_free(struct file_node *node);
#endif // TREE_H
