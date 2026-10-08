#include "Keybind.hpp"

#include "../imgui_internal.h"

#include <Windows.h>

#include <cstdio>

namespace Custom {

namespace {
	ImGuiID g_activeKeybindId = 0;
	bool g_waitForKeyRelease = false;

	bool IsAnyKeyDown() {
		for (int virtualKey = 0x01; virtualKey <= 0xFE; ++virtualKey) {
			if (GetAsyncKeyState(virtualKey) & 0x8000)
				return true;
		}

		return false;
	}
}

const char* GetKeyName(int virtualKey) {
	switch (virtualKey) {
	case 0: return "None";
	case VK_LBUTTON: return "Mouse 1";
	case VK_RBUTTON: return "Mouse 2";
	case VK_MBUTTON: return "Mouse 3";
	case VK_XBUTTON1: return "Mouse 4";
	case VK_XBUTTON2: return "Mouse 5";
	case VK_BACK: return "Backspace";
	case VK_TAB: return "Tab";
	case VK_RETURN: return "Enter";
	case VK_SHIFT: return "Shift";
	case VK_CONTROL: return "Ctrl";
	case VK_MENU: return "Alt";
	case VK_PAUSE: return "Pause";
	case VK_CAPITAL: return "Caps Lock";
	case VK_ESCAPE: return "Escape";
	case VK_SPACE: return "Space";
	case VK_PRIOR: return "Page Up";
	case VK_NEXT: return "Page Down";
	case VK_END: return "End";
	case VK_HOME: return "Home";
	case VK_LEFT: return "Left";
	case VK_UP: return "Up";
	case VK_RIGHT: return "Right";
	case VK_DOWN: return "Down";
	case VK_INSERT: return "Insert";
	case VK_DELETE: return "Delete";
	case VK_NUMPAD0: return "Num 0";
	case VK_NUMPAD1: return "Num 1";
	case VK_NUMPAD2: return "Num 2";
	case VK_NUMPAD3: return "Num 3";
	case VK_NUMPAD4: return "Num 4";
	case VK_NUMPAD5: return "Num 5";
	case VK_NUMPAD6: return "Num 6";
	case VK_NUMPAD7: return "Num 7";
	case VK_NUMPAD8: return "Num 8";
	case VK_NUMPAD9: return "Num 9";
	case VK_MULTIPLY: return "Num *";
	case VK_ADD: return "Num +";
	case VK_SUBTRACT: return "Num -";
	case VK_DECIMAL: return "Num .";
	case VK_DIVIDE: return "Num /";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	case VK_LSHIFT: return "Left Shift";
	case VK_RSHIFT: return "Right Shift";
	case VK_LCONTROL: return "Left Ctrl";
	case VK_RCONTROL: return "Right Ctrl";
	case VK_LMENU: return "Left Alt";
	case VK_RMENU: return "Right Alt";
	default: break;
	}

	if (virtualKey >= '0' && virtualKey <= '9') {
		static char digitName[2] = {};
		digitName[0] = static_cast<char>(virtualKey);
		digitName[1] = '\0';
		return digitName;
	}

	if (virtualKey >= 'A' && virtualKey <= 'Z') {
		static char letterName[2] = {};
		letterName[0] = static_cast<char>(virtualKey);
		letterName[1] = '\0';
		return letterName;
	}

	static char fallbackName[16];
	snprintf(fallbackName, sizeof(fallbackName), "0x%02X", virtualKey);
	return fallbackName;
}

bool Keybind(const char* label, int* key, const KeybindPreset& preset) {
	if (!key)
		return false;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiID id = window->GetID(label);
	const ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
	const float availWidth = ImGui::GetContentRegionAvail().x;
	const float boxWidth = availWidth - labelSize.x - preset.spacing;
	const float rowHeight = preset.rowHeight;

	const ImVec2 pos = window->DC.CursorPos;
	const ImRect totalBb(pos, ImVec2(pos.x + availWidth, pos.y + rowHeight));
	ImGui::ItemSize(totalBb);
	if (!ImGui::ItemAdd(totalBb, id))
		return false;

	const float boxX = pos.x + labelSize.x + preset.spacing;
	const float boxY = pos.y + (rowHeight - preset.boxHeight) * 0.5f;
	const ImRect boxBb(
		ImVec2(boxX, boxY),
		ImVec2(boxX + boxWidth, boxY + preset.boxHeight)
	);

	bool hovered = false;
	bool held = false;
	const bool pressed = ImGui::ButtonBehavior(boxBb, id, &hovered, &held);
	const bool rightClicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right);

	bool changed = false;
	const bool isListening = g_activeKeybindId == id;

	if (pressed) {
		g_activeKeybindId = id;
		g_waitForKeyRelease = true;
	}

	if (rightClicked) {
		*key = 0;
		g_activeKeybindId = 0;
		g_waitForKeyRelease = false;
		changed = true;
	}

	if (isListening) {
		if (g_waitForKeyRelease) {
			if (!IsAnyKeyDown())
				g_waitForKeyRelease = false;
		}
		else {
			for (int virtualKey = 0x01; virtualKey <= 0xFE; ++virtualKey) {
				if (!(GetAsyncKeyState(virtualKey) & 0x8000))
					continue;

				if (virtualKey == VK_ESCAPE) {
					g_activeKeybindId = 0;
					break;
				}

				*key = virtualKey;
				g_activeKeybindId = 0;
				changed = true;
				break;
			}
		}
	}

	ImDrawList* drawList = window->DrawList;

	const float labelY = pos.y + (rowHeight - labelSize.y) * 0.5f;
	drawList->AddText(ImVec2(pos.x, labelY), ColorToU32(preset.labelColor), label);

	const ImVec4 borderColor = isListening
		? preset.boxBorderListening
		: (hovered ? preset.boxBorderHovered : preset.boxBorder);

	drawList->AddRectFilled(
		boxBb.Min,
		boxBb.Max,
		ColorToU32(isListening ? preset.boxBackgroundListening : preset.boxBackground),
		preset.rounding
	);
	drawList->AddRect(
		boxBb.Min,
		boxBb.Max,
		ColorToU32(borderColor),
		preset.rounding,
		0,
		1.f
	);

	const char* valueText = isListening ? "..." : GetKeyName(*key);
	const ImVec2 valueSize = ImGui::CalcTextSize(valueText);
	const ImVec4 valueColor = isListening ? preset.valueListeningColor : preset.valueColor;
	const float valueY = boxBb.Min.y + (preset.boxHeight - valueSize.y) * 0.5f;
	const float valueX = boxBb.Min.x + (boxBb.GetWidth() - valueSize.x) * 0.5f;

	drawList->AddText(
		ImVec2(valueX, valueY),
		ColorToU32(valueColor),
		valueText
	);

	return changed;
}

} // namespace Custom
