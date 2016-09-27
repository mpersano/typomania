#pragma once

#include <map>
#include <memory>

struct pattern_node;

class kana_to_pattern
{
public:
	static const pattern_node *find_pair(const wchar_t first, const wchar_t second);
	static const pattern_node *find_single(const wchar_t kana);

private:
	static kana_to_pattern& get_instance();

	kana_to_pattern();

	using pattern_node_ptr = std::unique_ptr<pattern_node>;

	using kana_map = std::map<wchar_t, pattern_node_ptr>;
	kana_map kana_to_pattern_map;

	using kana_pair_map = std::map<std::pair<wchar_t, wchar_t>, pattern_node_ptr>;
	kana_pair_map kana_pair_to_pattern_map;
};
