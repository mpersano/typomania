#ifndef KANA_H_
#define KANA_H_

#include <map>

struct pattern_node;

class kana_to_pattern {
public:
	static pattern_node *find_pair(const wchar_t first, const wchar_t second);
	static pattern_node *find_single(const wchar_t kana);

private:
	static kana_to_pattern& get_instance();

	kana_to_pattern();
	~kana_to_pattern();

	typedef std::map<wchar_t, pattern_node *> kana_map;
	kana_map kana_to_pattern_map;

	typedef std::map<std::pair<wchar_t, wchar_t>, pattern_node *> kana_pair_map;
	kana_pair_map kana_pair_to_pattern_map;
};

#endif // KANA_H_
