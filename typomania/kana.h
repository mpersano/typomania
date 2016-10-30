#pragma once

#include <map>
#include <memory>

struct pattern_node;

namespace kana {

const pattern_node *find_pattern(wchar_t kana0, wchar_t kana1, wchar_t kana2);
const pattern_node *find_pattern(wchar_t kana0, wchar_t kana1);
const pattern_node *find_pattern(wchar_t kana);

}
