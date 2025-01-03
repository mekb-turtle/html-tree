#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

#include "html.h"

void html_print_escaped(const char *str, FILE *stream) {
	if (!str) return;

	// loop through string
	for (; *str; str++) {
		switch (*str) {
			// escape special characters
			case '<':
				fputs("&lt;", stream);
				break;
			case '>':
				fputs("&gt;", stream);
				break;
			case '&':
				fputs("&amp;", stream);
				break;
			case '"':
				fputs("&quot;", stream);
				break;
			case '\'':
				fputs("&#x27;", stream);
				break;
			case '\n':
			case '\r':
			case '\t':
				break;
			default:
				// print character as is
				fputc(*str, stream);
				break;
		}
	}
}


static void html_print_node(struct file_node *node, FILE *stream, struct options opts, size_t *count, size_t depth, char *classes) {
	if (!node) return;
	if (!count) return;

	// print node
	fputs("<tr class=\"node ", stream);
	if (classes) fputs(classes, stream);
	if (depth) fputs("child", stream);

	fprintf(stream, "\" id=\"item-%zu\">", *count);

	fputs("<td class=\"name\">", stream);

	// print depth indents
	for (size_t i = 0; i < depth; i++) fputs("<span class=\"indent\"></span>", stream);

	if (node->children) {
		// print separate arrow
		fputs("<span class=\"arrow\"></span>", stream);

		// details/summary for collapsible nodes
		fputs("<details class=\"node\"><summary>", stream);
	} else {
		fputs("<span class=\"no-arrow\"></span>", stream);
	}

	// print name
	fputs("<span class=\"name\">", stream);
	html_print_escaped(node->name, stream);
	fputs("</span>", stream);
	if (node->children) fputs("</summary></details>", stream);
	fputs("</td>", stream);

	// print size
	fputs("<td class=\"size number\" title=\"", stream);
	fprintf(stream, "%zu", node->size);
	fputs(" bytes\">", stream);
	print_size(node->size, opts.metric, stream);
	fputs("</td>", stream);

	// print type
	fputs("<td class=\"type\">", stream);
	if (S_ISREG(node->mode)) {
		fputs("File", stream);
	} else if (S_ISDIR(node->mode)) {
		fputs("Directory", stream);
	} else if (S_ISLNK(node->mode)) {
		fputs("Symbolic Link", stream);
	} else if (S_ISCHR(node->mode)) {
		fputs("Character Device", stream);
	} else if (S_ISBLK(node->mode)) {
		fputs("Block Device", stream);
	} else if (S_ISFIFO(node->mode)) {
		fputs("FIFO", stream);
	} else if (S_ISSOCK(node->mode)) {
		fputs("Socket", stream);
	} else {
		fputs("Unknown", stream);
	}
	fputs("</td>", stream);

	// print number of items
	fputs("<td class=\"items number\">", stream);
	if (node->children) {
		fprintf(stream, "%zu", node->num_items);
	} else if (node->target) {
		html_print_escaped(node->target, stream);
	}
	fputs("</td>", stream);

	// print mode
	char *octal = get_octal_mode(node->mode);
	char *symbolic = get_symbolic_mode(node->mode);

	fprintf(stream, "<td class=\"mode code%s\" title=\"", opts.alt_mode ? " number" : "");
	fputs(opts.alt_mode ? symbolic : octal, stream);
	fputs("\">", stream);
	fputs(opts.alt_mode ? octal : symbolic, stream);
	fputs("</td>", stream);

	// print owner
	struct passwd *pw = getpwuid(node->uid);
	fprintf(stream, "<td class=\"owner\" title=\"%u%s\">", node->uid, pw ? "" : " (not found)");
	if (pw) {
		html_print_escaped(pw->pw_name, stream);
	} else {
		fprintf(stream, "%u", node->uid);
	}
	fputs("</td>", stream);

	// print group
	struct group *gr = getgrgid(node->gid);
	fprintf(stream, "<td class=\"group\" title=\"%u%s\">", node->gid, gr ? "" : " (not found)");
	if (gr) {
		html_print_escaped(gr->gr_name, stream);
	} else {
		fprintf(stream, "%u", node->gid);
	}
	fputs("</td>", stream);


	fputs("</tr>", stream);

	// print children
	if (node->children) {
		// create identifier for children to show/hide them when parent is opened/closed
		size_t class_len = 10 + intlog10(*count) + (classes ? strlen(classes) : 0);
		char class[class_len];
		snprintf(class, class_len, "%schild-%zu ", classes ? classes : "", *count);

		// increment counter
		(*count)++;

		// print children
		for (struct file_node *child = node->children; child; child = child->next) {
			html_print_node(child, stream, opts, count, depth + 1, class);
		}
	} else {
		(*count)++;
	}
}

// embedded CSS
extern unsigned char style_css[];
extern unsigned int style_css_len;

void html_print_nodes(struct file_node *node, FILE *stream, struct options opts) {
	if (!node) return;
	if (!opts.title) opts.title = "Directory Tree";

	// print header
	fputs("<!DOCTYPE html><html><head><title>", stream);
	html_print_escaped(opts.title, stream);
	fputs("</title><style>", stream);
	fwrite(style_css, 1, style_css_len, stream); // write embedded CSS
	fputs("</style></head><body>", stream);

	// print tree
	fputs("<table class=\"tree\">", stream);
	fputs("<thead><tr>", stream);

	const char *headers[] = {"Name", "Size", "Type", "Items", "Mode", "Owner", "Group", NULL};

	// generate columns
	for (size_t i = 0; headers[i]; i++) {
		fprintf(stream, "<th>%s</th>", headers[i]);
	}

	fputs("</tr></thead><tbody>", stream);

	// print nodes
	size_t count = 0;
	for (; node; node = node->next)
		html_print_node(node, stream, opts, &count, 0, NULL);

	fputs("</tbody></table>", stream);

	// print CSS to show children when parent is opened
	// CSS does not have a previous sibling or a parent selector, so we have to generate a rule for each node
	// the only other alternative is to use JavaScript
	fputs("<style>", stream);
	for (size_t i = 0; i < count; i++) {
		fprintf(stream, "tr#item-%zu:has(> td > details:not([open])) ~ tr.child-%zu { display: none; }", i, i);
	}
	fputs("</style>", stream);

	// print footer
	fputs("</body></html>", stream);
}
