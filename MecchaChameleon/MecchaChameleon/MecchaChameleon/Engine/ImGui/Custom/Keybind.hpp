#pragma once

#include "Presets.hpp"

namespace Custom {

bool Keybind(
	const char* label,
	int* key,
	const KeybindPreset& preset = g_presets.keybind
);

const char* GetKeyName(int virtualKey);

} // namespace Custom
