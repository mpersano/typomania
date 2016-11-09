#pragma once

struct pattern_node;

namespace kana {

const pattern_node *find_pattern(wchar_t kana0, wchar_t kana1, wchar_t kana2);
const pattern_node *find_pattern(wchar_t kana0, wchar_t kana1);
const pattern_node *find_pattern(wchar_t kana);

template <typename Iterator>
std::pair<const pattern_node *, int> find_pattern(Iterator it)
{
	wchar_t c0 = *it;
	++it;

	wchar_t c1 = *it;
	++it;

	wchar_t c2 = *it;

	if (auto p = find_pattern(c0, c1, c2))
		return { p, 3 };

	if (auto p = find_pattern(c0, c1))
		return { p, 2 };

	if (auto p = find_pattern(c0))
		return { p, 1 };

	return { nullptr, 0 };
}

}
