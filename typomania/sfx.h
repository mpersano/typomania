#pragma once

namespace sfx {

enum class effect { MENU_BEEP, MENU_SELECTION };

void init();

int add_effect(const std::string& source, int max_sources);
void play(int effect_id);

};
