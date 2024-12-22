#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <libgen.h>

#include "tree.h"

// find where to insert new_node in nodes, if the list is sorted by the given function
static struct file_node *find_prev_node_by_order(struct file_node *nodes, struct file_node *new_node, bool (*compare)(struct file_node *, struct file_node *, struct options opts), struct options opts) {
	if (!nodes) return NULL;

	// loop through nodes
	struct file_node *prev = NULL;
	while (nodes) {
		// true if new_node should be inserted before current node
		if (compare(nodes, new_node, opts)) return prev;

		// move to next node
		if (!prev) prev = nodes;
		else
			prev = prev->next;
		nodes = prev->next;
	}
	return prev; // previous is at the end
}

static void insert_node_by_order(struct file_node **nodes, struct file_node *new_node, bool (*compare)(struct file_node *, struct file_node *, struct options opts), struct options opts) {
	if (!nodes) return;
	if (!new_node) return;

	// find previous node
	struct file_node *prev = find_prev_node_by_order(*nodes, new_node, compare, opts);

	if (!prev) {
		// insert at head
		new_node->next = *nodes;
		*nodes = new_node;
	} else {
		// insert after prev
		new_node->next = prev->next;
		prev->next = new_node;
	}
}

bool compare_files(struct file_node *list, struct file_node *new_node, struct options opts) {
	(void) opts;
	if (list->size == new_node->size) return strcmp(list->name, new_node->name) > 0; // compare name if sizes are equal

	if (opts.reverse) return list->size > new_node->size;
	return list->size < new_node->size;
}

static struct file_node *tree_build_internal(const char *path, struct options opts) {
	if (!path) return NULL;

	// stat file
	struct stat st;
	if (lstat(path, &st)) {
		warn("%s", path);
		return NULL;
	}

	// create node
	struct file_node *node = malloc(sizeof(struct file_node));
	if (!node) {
		warn("malloc");
		return NULL;
	}

	// set node values
	node->path = strdup(path); // copy path to avoid dangling pointer
	if (!node->path) {
		warn("strdup: %s", path);
		free(node);
		return NULL;
	}
	node->name = basename(node->path);
	node->size = st.st_size;
	node->num_items = 0;
	node->mode = st.st_mode;
	node->children = NULL;
	node->next = NULL;

	if (S_ISDIR(st.st_mode)) {
		node->size = 0;
		char resolved_path[PATH_MAX];
		// resolve directory path
		if (!realpath(path, resolved_path)) {
			warn("realpath: %s", path);
			return NULL;
		}

		// read directory
		DIR *dir = opendir(path);
		if (!dir) {
			warn("opendir: %s", path);
			free(node->path);
			free(node);
			return NULL;
		}

		struct dirent *entry;
		while ((entry = readdir(dir))) {
			if (!entry) break;
			if (entry->d_name[0] == '.') {
				if (!opts.dotfiles) continue;                                      // skip dotfiles
				if (entry->d_name[1] == '\0') continue;                            // skip "."
				if (entry->d_name[1] == '.' && entry->d_name[2] == '\0') continue; // skip ".."
			}

			// check if path is too long
			if (strlen(resolved_path) + strlen(entry->d_name) + 2 > PATH_MAX) {
				warnx("path too long: %s/%s", resolved_path, entry->d_name);
				continue;
			}

			char new_path[PATH_MAX];
			strcpy(new_path, resolved_path);
			// append item name
			strcat(new_path, "/");
			strcat(new_path, entry->d_name);

			struct file_node *new_child = tree_build_internal(new_path, opts);
			if (!new_child) continue;

			// add size and num items to parent
			node->size += new_child->size;
			node->num_items += new_child->num_items; // add the child's descendants
			++node->num_items;                       // add the actual child itself

			// insert child
			insert_node_by_order(&node->children, new_child, compare_files, opts);
		}

		closedir(dir);
	}

	return node;
}

struct file_node *tree_build(const char *path, struct options opts) {
	// resolve path before building tree
	// this is done only for the root node to avoid following symlinks for every node
	char resolved_path[PATH_MAX];
	if (!realpath(path, resolved_path)) {
		warn("realpath: %s", path);
		return NULL;
	}

	return tree_build_internal(resolved_path, opts);
}

void tree_free(struct file_node *node) {
	if (!node) return;

	// loop through next nodes
	for (struct file_node *next; node; node = next) {
		next = node->next;

		tree_free(node->children); // recursively free children
		free(node->path);          // free name
		free(node);                // free node
	}
}
