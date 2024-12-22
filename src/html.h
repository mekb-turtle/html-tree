#ifndef HTML_H
#define HTML_H

#include <stdio.h>

#include "tree.h"
#include "util.h"

void html_print_escaped(const char *str, FILE *stream);
void html_print_nodes(struct file_node *node, FILE *stream, struct options opts);
#endif // HTML_H
