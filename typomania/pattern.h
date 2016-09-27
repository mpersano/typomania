#pragma once

#include <vector>
#include <algorithm>

struct pattern_node
{
	pattern_node() : is_optional(false), next(0) { }
	virtual ~pattern_node() { if (next) delete next; }

	virtual bool match(int keysym) const = 0;
	virtual int get_char() const = 0;

	bool is_optional;
	pattern_node *next;
};

struct single_char_node : pattern_node
{
	single_char_node(int ch) : ch(ch) { }

	bool match(int keysym) const
	{ return ch == keysym; }

	int get_char() const
	{ return ch; }

	int ch;
};

struct multi_char_node : pattern_node
{
	bool match(int keysym) const
	{ return std::find(char_list.begin(), char_list.end(), keysym) != char_list.end(); }

	int get_char() const
	{ return char_list[0]; }

	std::vector<int> char_list;
};

pattern_node *
parse_pattern(const char *pattern_str);
