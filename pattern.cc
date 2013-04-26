#include "pattern.h"

pattern_node *
parse_pattern(const char *pattern_str)
{
	pattern_node *head = 0, **node_ptr = &head;

	while (*pattern_str) {
		pattern_node *node;

		int ch = *pattern_str++;

		if (ch == '[') {
			node = new multi_char_node;

			while ((ch = *pattern_str++) != ']' && ch != '\0')
				dynamic_cast<multi_char_node *>(node)->char_list.push_back(ch);
		} else {
			node = new single_char_node(ch);
		}

		if (*pattern_str == '?') {
			node->is_optional = true;
			++pattern_str;
		}

		*node_ptr = node;
		node_ptr = &node->next;
	}

	return head;
}
