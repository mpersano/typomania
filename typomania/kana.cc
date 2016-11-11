#include <map>
#include <memory>

#include "pattern.h"
#include "kana.h"

namespace kana {

namespace {

class kana_to_pattern
{
public:
	static kana_to_pattern& get_instance();

	const pattern_node *find_pattern(wchar_t kana);
	const pattern_node *find_pattern(wchar_t kana0, wchar_t kana1);
	const pattern_node *find_pattern(wchar_t kana0, wchar_t kana1, wchar_t kana2);

private:
	kana_to_pattern();

	std::map<wchar_t, std::shared_ptr<pattern_node>> kana_to_pattern_;
	std::map<std::pair<wchar_t, wchar_t>, std::shared_ptr<pattern_node>> kana_pair_to_pattern_;
	std::map<std::tuple<wchar_t, wchar_t, wchar_t>, std::shared_ptr<pattern_node>> kana_triple_to_pattern_;
};

wchar_t
hira_to_kata(wchar_t ch)
{
	return ch - L'あ' + L'ア';
}

wchar_t
half_to_full(wchar_t ch)
{
	return ch - L'a' + L'ａ';
}

kana_to_pattern::kana_to_pattern()
{
	static const std::vector<std::pair<wchar_t, const char *>> kana_to_romaji
		{ { L'あ', "A"  }, { L'い', "I"  }, { L'う', "U"  }, { L'え', "E"  }, { L'お', "O"  },
		  { L'か', "KA" }, { L'き', "KI" }, { L'く', "KU" }, { L'け', "KE" }, { L'こ', "KO" },
		  { L'さ', "SA" }, { L'し', "SH?I" }, { L'す', "SU" }, { L'せ', "SE" }, { L'そ', "SO" },
		  { L'た', "TA" }, { L'ち', "[TC]H?I" }, { L'つ', "TS?U" }, { L'て', "TE" }, { L'と', "TO" },
		  { L'な', "NA" }, { L'に', "NI" }, { L'ぬ', "NU" }, { L'ね', "NE" }, { L'の', "NO" },
		  { L'は', "HA" }, { L'ひ', "HI" }, { L'ふ', "[HF]U" }, { L'へ', "HE" }, { L'ほ', "HO" },
		  { L'ま', "MA" }, { L'み', "MI" }, { L'む', "MU" }, { L'め', "ME" }, { L'も', "MO" },
		  { L'や', "YA" }, { L'ゆ', "YU" }, { L'よ', "YO" },
		  { L'ら', "RA" }, { L'り', "RI" }, { L'る', "RU" }, { L'れ', "RE" }, { L'ろ', "RO" },
		  { L'わ', "WA" }, { L'を', "WO" },
		  { L'ん', "N"  }, { L'ー', "-" },
		  { L'が', "GA" }, { L'ぎ', "GI" }, { L'ぐ', "GU" }, { L'げ', "GE" }, { L'ご', "GO" },
		  { L'ざ', "ZA" }, { L'じ', "[ZJ]I" }, { L'ず', "ZU" }, { L'ぜ', "ZE" }, { L'ぞ', "ZO" },
		  { L'だ', "DA" }, { L'ぢ', "DI" }, { L'づ', "[DZ]U" }, { L'で', "DE" }, { L'ど', "DO" },
		  { L'ば', "BA" }, { L'び', "BI" }, { L'ぶ', "BU" }, { L'べ', "BE" }, { L'ぼ', "BO" },
		  { L'ぱ', "PA" }, { L'ぴ', "PI" }, { L'ぷ', "PU" }, { L'ぺ', "PE" }, { L'ぽ', "PO" },
		  { L'ぁ', "[LX]A"  }, { L'ぃ', "[LX]I"  }, { L'ぅ', "[LX]U"  }, { L'ぇ', "[LX]E"  }, { L'ぉ', "[LX]O"  },
		  { L'っ', "[LX]TS?U" } };

	for (auto& p : kana_to_romaji) {
		std::shared_ptr<pattern_node> pattern(parse_pattern(p.second));

		kana_to_pattern_[p.first] = pattern;
		kana_to_pattern_[hira_to_kata(p.first)] = pattern;
	}

	for (char i = 'A'; i <= 'Z'; i++) {
		char pattern_str[] { i, '\0' };
		std::shared_ptr<pattern_node> pattern(parse_pattern(pattern_str));

		kana_to_pattern_[i] = pattern;
		kana_to_pattern_[half_to_full(i)] = pattern;
	}

	for (char i = 'a'; i <= 'z'; i++) {
		char pattern_str[] { static_cast<char>(i - 'a' + 'A'), '\0' };
		std::shared_ptr<pattern_node> pattern(parse_pattern(pattern_str));

		kana_to_pattern_[i] = pattern;
		kana_to_pattern_[half_to_full(i)] = pattern;
	}

	for (char i = '0'; i <= '9'; i++) {
		char pattern_str[] { i, '\0' };
		std::shared_ptr<pattern_node> pattern(parse_pattern(pattern_str));

		kana_to_pattern_[i] = pattern;
		kana_to_pattern_[half_to_full(i)] = pattern;
	}

	static const std::vector<std::pair<const wchar_t *, const char *>> kana_pair_to_romaji
		{ { L"きゃ", "KYA" }, { L"きゅ", "KYU" }, { L"きょ", "KYO" },
		  { L"しゃ", "S[YH]A" }, { L"しゅ", "S[YH]U" }, { L"しょ", "S[YH]O" },
		  { L"ちゃ", "[TC][YH]A" }, { L"ちゅ", "[TC][YH]U" }, { L"ちょ", "[TC][YH]O" },
		  { L"にゃ", "NYA" }, { L"にゅ", "NYU" }, { L"にょ", "NYO" },
		  { L"ひゃ", "HYA" }, { L"ひゅ", "HYU" }, { L"ひょ", "HYO" },
		  { L"みゃ", "MYA" }, { L"みゅ", "MYU" }, { L"みょ", "MYO" },
		  { L"りゃ", "RYA" }, { L"りゅ", "RYU" }, { L"りょ", "RYO" },
		  { L"ぎゃ", "GYA" }, { L"ぎゅ", "GYU" }, { L"ぎょ", "GYO" },
		  { L"じゃ", "[JZ]Y?A"  }, { L"じゅ", "[JZ]Y?U"  }, { L"じょ", "[JZ]Y?O"  }, // XXX:should display ZYA/ZYU/ZYO for consistency
		  { L"びゃ", "BYA" }, { L"びゅ", "BYU" }, { L"びょ", "BYO" },
		  { L"ぴゃ", "PYA" }, { L"ぴゅ", "PYU" }, { L"ぴょ", "PYO" },
		  { L"った", "TTA" }, { L"っち", "TTI" }, { L"っつ", "TTU" }, { L"って", "TTE" }, { L"っと", "TTO" },
		  { L"っさ", "SSA" }, { L"っし", "SSI" }, { L"っす", "SSU" }, { L"っせ", "SSE" }, { L"っそ", "SSO" },
		  { L"っか", "KKA" }, { L"っき", "KKI" }, { L"っく", "KKU" }, { L"っけ", "KKE" }, { L"っこ", "KKO" },
		  { L"っぱ", "PPA" }, { L"っぴ", "PPI" }, { L"っぷ", "PPU" }, { L"っぺ", "PPE" }, { L"っぽ", "PPO" } };

	for (auto& p : kana_pair_to_romaji) {
		std::shared_ptr<pattern_node> pattern(parse_pattern(p.second));

		auto kana = p.first;

		kana_pair_to_pattern_[std::make_pair(kana[0], kana[1])] = pattern;
		kana_pair_to_pattern_[std::make_pair(hira_to_kata(kana[0]), hira_to_kata(kana[1]))] = pattern;
	}

	static const std::vector<std::pair<const wchar_t *, const char *>> kana_triple_to_romaji
		{ { L"っきゃ", "KKYA" }, { L"っきゅ", "KKYU" }, { L"っきょ", "KKYO" },
		  { L"っしゃ", "SS[YH]A" }, { L"っしゅ", "SS[YH]U" }, { L"っしょ", "SS[YH]O" },
		  { L"っちゃ", "[TC][TC][YH]A" }, { L"っちゅ", "[TC][TC][YH]U" }, { L"っちょ", "[TC][TC][YH]O" },
		  { L"っひゃ", "HHYA" }, { L"っひゅ", "HHYU" }, { L"っひょ", "HHYO" },
		  { L"っみゃ", "MMYA" }, { L"っみゅ", "MMYU" }, { L"っみょ", "MMYO" },
		  { L"っりゃ", "RRYA" }, { L"っりゅ", "RRYU" }, { L"っりょ", "RRYO" },
		  { L"っぎゃ", "GGYA" }, { L"っぎゅ", "GGYU" }, { L"っぎょ", "GGYO" },
		  { L"っじゃ", "[JZ][JZ]Y?A"  }, { L"っじゅ", "[JZ][JZ]Y?U"  }, { L"っじょ", "[JZ][JZ]Y?O"  }, // XXX:should display ZYA/ZYU/ZYO for consistency
		  { L"っびゃ", "BBYA" }, { L"っびゅ", "BBYU" }, { L"っびょ", "BBYO" },
		  { L"っぴゃ", "PPYA" }, { L"っぴゅ", "PPYU" }, { L"っぴょ", "PPYO" } };

	for (auto& p : kana_triple_to_romaji) {
		std::shared_ptr<pattern_node> pattern(parse_pattern(p.second));

		auto kana = p.first;

		kana_triple_to_pattern_[std::make_tuple(p.first[0], p.first[1], p.first[2])] = pattern;
		kana_triple_to_pattern_[std::make_tuple(hira_to_kata(p.first[0]), hira_to_kata(p.first[1]), hira_to_kata(p.first[2]))] = pattern;
	}

}

kana_to_pattern&
kana_to_pattern::get_instance()
{
	static kana_to_pattern instance;
	return instance;
}

const pattern_node *
kana_to_pattern::find_pattern(wchar_t kana0, wchar_t kana1, wchar_t kana2)
{
	auto it = kana_triple_to_pattern_.find(std::make_tuple(kana0, kana1, kana2));
	return it != kana_triple_to_pattern_.end() ? it->second.get() : nullptr;
}

const pattern_node *
kana_to_pattern::find_pattern(wchar_t kana0, wchar_t kana1)
{
	auto it = kana_pair_to_pattern_.find(std::make_pair(kana0, kana1));
	return it != kana_pair_to_pattern_.end() ? it->second.get() : nullptr;
}

const pattern_node *
kana_to_pattern::find_pattern(wchar_t kana)
{
	auto it = kana_to_pattern_.find(kana);
	return it != kana_to_pattern_.end() ? it->second.get() : nullptr;
}

}

const pattern_node *
find_pattern(wchar_t kana)
{
	kana_to_pattern& instance = kana_to_pattern::get_instance();
	return instance.find_pattern(kana);
}

const pattern_node *
find_pattern(wchar_t kana0, wchar_t kana1)
{
	kana_to_pattern& instance = kana_to_pattern::get_instance();
	return instance.find_pattern(kana0, kana1);
}

const pattern_node *
find_pattern(wchar_t kana0, wchar_t kana1, wchar_t kana2)
{
	kana_to_pattern& instance = kana_to_pattern::get_instance();
	return instance.find_pattern(kana0, kana1, kana2);
}

}
