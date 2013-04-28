#include "pattern.h"
#include "kana.h"

kana_to_pattern::kana_to_pattern()
{
	static const struct kana_to_romaji {
		const wchar_t kana;
		const char *romaji;
	} kana_to_romaji_table[] = {
		{ L'あ', "A"  }, { L'い', "I"  }, { L'う', "U"  }, { L'え', "E"  }, { L'お', "O"  },
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
		{ 0, 0 },
	};

	for (const kana_to_romaji *p = kana_to_romaji_table; p->kana; p++)
		kana_to_pattern_map[p->kana] = parse_pattern(p->romaji);

	for (int i = 'A'; i <= 'Z'; i++) {
		char pattern[2] = { i, '\0' };
		kana_to_pattern_map[i] = parse_pattern(pattern);
	}

	for (int i = 'a'; i <= 'z'; i++) {
		char pattern[2] = { i - 'a' + 'A', '\0' };
		kana_to_pattern_map[i] = parse_pattern(pattern);
	}

	for (int i = '0'; i <= '9'; i++) {
		char pattern[2] = { i, '\0' };
		kana_to_pattern_map[i] = parse_pattern(pattern);
	}

	static const struct kana_pair_to_romaji {
		const wchar_t *kana;
		const char *romaji;
	} kana_pair_to_romaji_table[] = {
		{ L"きゃ", "KYA" }, { L"きゅ", "KYU" }, { L"きょ", "KYO" },
		{ L"しゃ", "S[YH]A" }, { L"しゅ", "S[YH]U" }, { L"しょ", "S[YH]O" },
		{ L"ちゃ", "[TC][YH]A" }, { L"ちゅ", "[TC][YH]U" }, { L"ちょ", "[TC][YH]O" },
		{ L"にゃ", "NYA" }, { L"にゅ", "NYU" }, { L"にょ", "NYO" },
		{ L"ひゃ", "HYA" }, { L"ひゅ", "HYU" }, { L"ひょ", "HYO" },
		{ L"みゃ", "MYA" }, { L"みゅ", "MYU" }, { L"みょ", "MYO" },
		{ L"りゃ", "RYA" }, { L"りゅ", "RYU" }, { L"りょ", "RYO" },
		{ L"ぎゃ", "GYA" }, { L"ぎゅ", "GYU" }, { L"ぎょ", "GYO" },
		{ L"じゃ", "[JZ]Y?A"  }, { L"じゅ", "[JZ]Y?U"  }, { L"じょ", "[JZ]Y?O"  }, // should display ZYA/ZYU/ZYO for consistency
		{ L"びゃ", "BYA" }, { L"びゅ", "BYU" }, { L"びょ", "BYO" },
		{ L"ぴゃ", "PYA" }, { L"ぴゅ", "PYU" }, { L"ぴょ", "PYO" },
		{ L"った", "TTA" }, { L"っち", "TTI" }, { L"っつ", "TTU" }, { L"って", "TTE" }, { L"っと", "TTO" },
		{ L"っさ", "SSA" }, { L"っし", "SSI" }, { L"っす", "SSU" }, { L"っせ", "SSE" }, { L"っそ", "SSO" },
		{ L"っか", "KKA" }, { L"っき", "KKI" }, { L"っく", "KKU" }, { L"っけ", "KKE" }, { L"っこ", "KKO" },
		{ 0, 0 }
	};

	for (const kana_pair_to_romaji *p = kana_pair_to_romaji_table; p->kana; p++)
		kana_pair_to_pattern_map[std::pair<wchar_t, wchar_t>(p->kana[0], p->kana[1])] = parse_pattern(p->romaji);
}

kana_to_pattern::~kana_to_pattern()
{
	for (kana_map::iterator i = kana_to_pattern_map.begin(); i != kana_to_pattern_map.end(); i++)
		delete i->second;

	for (kana_pair_map::iterator i = kana_pair_to_pattern_map.begin(); i != kana_pair_to_pattern_map.end(); i++)
		delete i->second;
}

pattern_node *
kana_to_pattern::find_single(const wchar_t kana)
{
	kana_to_pattern& instance = get_instance();
	kana_map::iterator i = instance.kana_to_pattern_map.find(kana);
	return i != instance.kana_to_pattern_map.end() ? i->second : 0;
}

pattern_node *
kana_to_pattern::find_pair(wchar_t first, wchar_t second)
{
	kana_to_pattern& instance = get_instance();
	kana_pair_map::iterator i = instance.kana_pair_to_pattern_map.find(std::pair<wchar_t, wchar_t>(first, second));
	return i != instance.kana_pair_to_pattern_map.end() ? i->second : 0;
}
