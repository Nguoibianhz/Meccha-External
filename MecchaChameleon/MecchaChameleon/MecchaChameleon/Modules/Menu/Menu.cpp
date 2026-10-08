#include "Menu.hpp"

#include "../../Manager/Globals/Globals.hpp"
#include "../Overlay/Overlay.hpp"
#include "../../Engine/ImGui/imgui.h"
#include "../../Engine/ImGui/imgui_internal.h"
#include "../../Engine/ImGui/Custom/Presets.hpp"
#include "../../Engine/ImGui/Custom/ColorPicker.hpp"

#include <cmath>
#include <cstdio>
#include <algorithm>

namespace {

// =========================================================================
// Palette / Theme Constants (Exact dark slate theme from user mockup)
// =========================================================================
constexpr ImU32 kColWindowBg       = IM_COL32(11, 13, 17, 255);
constexpr ImU32 kColBorder         = IM_COL32(30, 34, 45, 255);
constexpr ImU32 kColHeaderBorder   = IM_COL32(24, 27, 36, 255);
constexpr ImU32 kColCardBg         = IM_COL32(17, 19, 26, 255);
constexpr ImU32 kColCardBorder     = IM_COL32(30, 35, 48, 255);
constexpr ImU32 kColCardHoverBorder= IM_COL32(45, 52, 70, 255);
constexpr ImU32 kColNavActiveBg    = IM_COL32(28, 32, 43, 255);
constexpr ImU32 kColNavHoverBg     = IM_COL32(21, 24, 33, 255);
constexpr ImU32 kColTextWhite      = IM_COL32(245, 247, 250, 255);
constexpr ImU32 kColTextGray       = IM_COL32(150, 156, 170, 255);
constexpr ImU32 kColTextMuted      = IM_COL32(110, 116, 130, 255);
constexpr ImU32 kColControlBg      = IM_COL32(23, 26, 35, 255);
constexpr ImU32 kColControlBorder  = IM_COL32(42, 47, 62, 255);
constexpr ImU32 kColControlHover   = IM_COL32(60, 68, 88, 255);
constexpr ImU32 kColAccentBlue     = IM_COL32(75, 120, 240, 255);
constexpr ImU32 kColIndicatorDot   = IM_COL32(240, 245, 255, 255);

// =========================================================================
// Procedural Vector Icons (ImDrawList sharp rendering at any DPI)
// =========================================================================

void DrawLogo(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	// Stylized 4-petal diamond / folded rhombus knot from mockup
	const float h = size * 0.5f;
	const float q = h * 0.45f;
	const float t = h * 0.20f;

	// Top leaf
	dl->AddQuadFilled(
		ImVec2(center.x - q, center.y - h),
		ImVec2(center.x + q, center.y - h + t),
		ImVec2(center.x + t, center.y - q),
		ImVec2(center.x - h + t, center.y - q),
		col
	);
	// Right leaf
	dl->AddQuadFilled(
		ImVec2(center.x + q, center.y - q),
		ImVec2(center.x + h, center.y + q),
		ImVec2(center.x + h - t, center.y + h - t),
		ImVec2(center.x + q - t, center.y + q),
		col
	);
	// Bottom leaf
	dl->AddQuadFilled(
		ImVec2(center.x - t, center.y + q),
		ImVec2(center.x + q, center.y + h - t),
		ImVec2(center.x - q, center.y + h),
		ImVec2(center.x - h + t, center.y + q),
		col
	);
	// Left leaf
	dl->AddQuadFilled(
		ImVec2(center.x - h, center.y - q),
		ImVec2(center.x - q, center.y - t),
		ImVec2(center.x - q, center.y + q),
		ImVec2(center.x - h + t, center.y + q - t),
		col
	);
}

void DrawCrosshairIcon(ImDrawList* dl, ImVec2 center, float r, ImU32 col) {
	dl->AddCircle(center, r * 0.75f, col, 16, 1.5f);
	dl->AddCircleFilled(center, 1.5f, col);
	dl->AddLine(ImVec2(center.x, center.y - r), ImVec2(center.x, center.y - r * 0.45f), col, 1.5f);
	dl->AddLine(ImVec2(center.x, center.y + r * 0.45f), ImVec2(center.x, center.y + r), col, 1.5f);
	dl->AddLine(ImVec2(center.x - r, center.y), ImVec2(center.x - r * 0.45f, center.y), col, 1.5f);
	dl->AddLine(ImVec2(center.x + r * 0.45f, center.y), ImVec2(center.x + r, center.y), col, 1.5f);
}

void DrawEyeIcon(ImDrawList* dl, ImVec2 center, float w, ImU32 col) {
	const float h = w * 0.55f;
	// Upper arc
	dl->AddBezierQuadratic(
		ImVec2(center.x - w, center.y),
		ImVec2(center.x, center.y - h),
		ImVec2(center.x + w, center.y),
		col, 1.5f
	);
	// Lower arc
	dl->AddBezierQuadratic(
		ImVec2(center.x - w, center.y),
		ImVec2(center.x, center.y + h),
		ImVec2(center.x + w, center.y),
		col, 1.5f
	);
	// Pupil
	dl->AddCircleFilled(center, h * 0.5f, col);
	dl->AddCircleFilled(ImVec2(center.x + 1.2f, center.y - 1.2f), h * 0.18f, IM_COL32(20, 24, 32, 255));
}

void DrawGearIcon(ImDrawList* dl, ImVec2 center, float r, ImU32 col) {
	dl->AddCircle(center, r * 0.7f, col, 16, 1.5f);
	dl->AddCircle(center, r * 0.35f, col, 12, 1.3f);
	for (int i = 0; i < 8; ++i) {
		const float angle = i * (3.14159265f / 4.0f);
		const float c = cosf(angle);
		const float s = sinf(angle);
		dl->AddLine(
			ImVec2(center.x + c * (r * 0.65f), center.y + s * (r * 0.65f)),
			ImVec2(center.x + c * (r * 0.95f), center.y + s * (r * 0.95f)),
			col, 2.0f
		);
	}
}

void DrawUserIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	// Head
	dl->AddCircle(ImVec2(center.x, center.y - size * 0.25f), size * 0.30f, col, 16, 1.5f);
	// Shoulders
	dl->AddBezierQuadratic(
		ImVec2(center.x - size * 0.55f, center.y + size * 0.5f),
		ImVec2(center.x, center.y + size * 0.15f),
		ImVec2(center.x + size * 0.55f, center.y + size * 0.5f),
		col, 1.5f
	);
}

void DrawCubeIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	const float s = size * 0.55f;
	const float dy = s * 0.5f;

	const ImVec2 top(center.x, center.y - s);
	const ImVec2 right(center.x + s * 0.866f, center.y - dy * 0.5f);
	const ImVec2 bottom(center.x, center.y + dy);
	const ImVec2 left(center.x - s * 0.866f, center.y - dy * 0.5f);
	const ImVec2 mid(center.x, center.y);
	const ImVec2 btmExt(center.x, center.y + s);

	// Hexagon outer outline
	dl->AddLine(top, right, col, 1.4f);
	dl->AddLine(right, ImVec2(right.x, right.y + s * 0.75f), col, 1.4f);
	dl->AddLine(ImVec2(right.x, right.y + s * 0.75f), btmExt, col, 1.4f);
	dl->AddLine(btmExt, ImVec2(left.x, left.y + s * 0.75f), col, 1.4f);
	dl->AddLine(ImVec2(left.x, left.y + s * 0.75f), left, col, 1.4f);
	dl->AddLine(left, top, col, 1.4f);

	// Inner 3 edges
	dl->AddLine(mid, top, col, 1.4f);
	dl->AddLine(mid, ImVec2(right.x, right.y + s * 0.75f), col, 1.4f);
	dl->AddLine(mid, ImVec2(left.x, left.y + s * 0.75f), col, 1.4f);
}

void DrawFunnelIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	const float s = size * 0.5f;
	const ImVec2 p1(center.x - s, center.y - s * 0.7f);
	const ImVec2 p2(center.x + s, center.y - s * 0.7f);
	const ImVec2 p3(center.x + s * 0.25f, center.y + s * 0.05f);
	const ImVec2 p4(center.x + s * 0.25f, center.y + s * 0.8f);
	const ImVec2 p5(center.x - s * 0.25f, center.y + s * 0.8f);
	const ImVec2 p6(center.x - s * 0.25f, center.y + s * 0.05f);

	dl->AddLine(p1, p2, col, 1.5f);
	dl->AddLine(p2, p3, col, 1.5f);
	dl->AddLine(p3, p4, col, 1.5f);
	dl->AddLine(p4, p5, col, 1.5f);
	dl->AddLine(p5, p6, col, 1.5f);
	dl->AddLine(p6, p1, col, 1.5f);
}

void DrawPaletteIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	const float r = size * 0.48f;
	dl->AddCircle(center, r, col, 16, 1.3f);
	// 3 tiny dots
	dl->AddCircleFilled(ImVec2(center.x - r * 0.35f, center.y - r * 0.3f), 1.2f, col);
	dl->AddCircleFilled(ImVec2(center.x + r * 0.25f, center.y - r * 0.35f), 1.2f, col);
	dl->AddCircleFilled(ImVec2(center.x - r * 0.30f, center.y + r * 0.25f), 1.2f, col);
	// Thumb hole
	dl->AddCircleFilled(ImVec2(center.x + r * 0.30f, center.y + r * 0.25f), 2.0f, IM_COL32(17, 19, 26, 255));
}

void DrawKeyboardIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col) {
	const float w = size * 0.85f;
	const float h = size * 0.55f;
	const ImVec2 kbMin(center.x - w * 0.5f, center.y - h * 0.5f);
	const ImVec2 kbMax(center.x + w * 0.5f, center.y + h * 0.5f);
	dl->AddRect(kbMin, kbMax, col, 2.5f, 0, 1.2f);
	// Key dots
	for (int r = 0; r < 2; ++r) {
		for (int c = 0; c < 4; ++c) {
			const float kx = kbMin.x + 3.0f + c * 3.2f;
			const float ky = kbMin.y + 2.5f + r * 2.8f;
			dl->AddRectFilled(ImVec2(kx, ky), ImVec2(kx + 1.8f, ky + 1.6f), col);
		}
	}
}

void DrawMoreDotsIcon(ImDrawList* dl, ImVec2 center, float r, ImU32 col) {
	dl->AddCircleFilled(ImVec2(center.x - 5.0f, center.y), r, col);
	dl->AddCircleFilled(center, r, col);
	dl->AddCircleFilled(ImVec2(center.x + 5.0f, center.y), r, col);
}

// =========================================================================
// Custom UI Controls matching the mockup exactly
// =========================================================================

bool CustomCheckbox(const char* label, bool* v) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return false;

	const ImGuiID id = window->GetID(label);
	const ImVec2 labelSize = ImGui::CalcTextSize(label, nullptr, true);
	const float boxSize = 16.0f;
	const float rowHeight = 26.0f;
	const ImVec2 pos = window->DC.CursorPos;

	const ImRect totalBb(pos, ImVec2(pos.x + boxSize + 8.0f + labelSize.x, pos.y + rowHeight));
	ImGui::ItemSize(totalBb);
	if (!ImGui::ItemAdd(totalBb, id)) return false;

	bool hovered = false, held = false;
	const bool pressed = ImGui::ButtonBehavior(totalBb, id, &hovered, &held);
	if (pressed) *v = !*v;

	const float boxY = pos.y + (rowHeight - boxSize) * 0.5f;
	const ImVec2 boxMin(pos.x, boxY);
	const ImVec2 boxMax(pos.x + boxSize, boxY + boxSize);
	ImDrawList* dl = window->DrawList;

	// Box outline & fill
	const ImU32 boxBg = *v ? IM_COL32(245, 247, 250, 255) : (hovered ? kColControlHover : kColControlBg);
	const ImU32 borderCol = hovered ? IM_COL32(80, 90, 115, 255) : kColControlBorder;

	if (*v) {
		dl->AddRectFilled(boxMin, boxMax, boxBg, 3.0f);
		// Checkmark
		const ImVec2 p1(boxMin.x + 3.5f, boxMin.y + 8.0f);
		const ImVec2 p2(boxMin.x + 6.5f, boxMin.y + 11.5f);
		const ImVec2 p3(boxMin.x + 12.5f, boxMin.y + 4.5f);
		dl->AddLine(p1, p2, IM_COL32(11, 13, 17, 255), 2.0f);
		dl->AddLine(p2, p3, IM_COL32(11, 13, 17, 255), 2.0f);
	} else {
		dl->AddRectFilled(boxMin, boxMax, boxBg, 3.0f);
		dl->AddRect(boxMin, boxMax, borderCol, 3.0f, 0, 1.2f);
	}

	const float textY = pos.y + (rowHeight - labelSize.y) * 0.5f;
	dl->AddText(ImVec2(boxMax.x + 8.0f, textY), kColTextWhite, label);
	return pressed;
}

bool CompactDropdown(const char* id, int* currentIdx, const char* const* items, int count, float width = 80.0f) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return false;

	const ImGuiID widgetId = window->GetID(id);
	const ImVec2 pos = window->DC.CursorPos;
	const float height = 24.0f;
	const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, widgetId)) return false;

	bool hovered = false, held = false;
	const bool pressed = ImGui::ButtonBehavior(bb, widgetId, &hovered, &held);

	char popupId[64];
	snprintf(popupId, sizeof(popupId), "##pop_%s", id);
	if (pressed) {
		ImGui::OpenPopup(popupId);
	}

	ImDrawList* dl = window->DrawList;
	const ImU32 bg = hovered ? kColControlHover : kColControlBg;
	dl->AddRectFilled(bb.Min, bb.Max, bg, 4.0f);
	dl->AddRect(bb.Min, bb.Max, kColControlBorder, 4.0f, 0, 1.0f);

	const char* preview = (*currentIdx >= 0 && *currentIdx < count) ? items[*currentIdx] : "";
	const ImVec2 textSize = ImGui::CalcTextSize(preview);
	dl->AddText(
		ImVec2(bb.Min.x + 8.0f, bb.Min.y + (height - textSize.y) * 0.5f),
		kColTextWhite, preview
	);

	// Chevron down
	const float arrowX = bb.Max.x - 12.0f;
	const float arrowY = bb.Min.y + height * 0.5f;
	dl->AddLine(ImVec2(arrowX - 3.5f, arrowY - 2.0f), ImVec2(arrowX, arrowY + 1.5f), kColTextGray, 1.5f);
	dl->AddLine(ImVec2(arrowX, arrowY + 1.5f), ImVec2(arrowX + 3.5f, arrowY - 2.0f), kColTextGray, 1.5f);

	bool changed = false;
	if (ImGui::BeginPopup(popupId)) {
		for (int i = 0; i < count; ++i) {
			const bool isSelected = (i == *currentIdx);
			if (ImGui::Selectable(items[i], isSelected)) {
				*currentIdx = i;
				changed = true;
			}
		}
		ImGui::EndPopup();
	}

	return changed;
}

bool PaletteIconButton(const char* id, ImVec4* colorToEdit) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return false;

	const ImGuiID widgetId = window->GetID(id);
	const ImVec2 pos = window->DC.CursorPos;
	const float size = 24.0f;
	const ImRect bb(pos, ImVec2(pos.x + size, pos.y + size));

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, widgetId)) return false;

	bool hovered = false, held = false;
	const bool pressed = ImGui::ButtonBehavior(bb, widgetId, &hovered, &held);

	char popupId[64];
	snprintf(popupId, sizeof(popupId), "##pal_%s", id);
	if (pressed) {
		ImGui::OpenPopup(popupId);
	}

	ImDrawList* dl = window->DrawList;
	const ImU32 bg = hovered ? kColControlHover : kColControlBg;
	dl->AddRectFilled(bb.Min, bb.Max, bg, 4.0f);
	dl->AddRect(bb.Min, bb.Max, kColControlBorder, 4.0f, 0, 1.0f);

	const ImVec2 center(bb.Min.x + size * 0.5f, bb.Min.y + size * 0.5f);
	DrawPaletteIcon(dl, center, 14.0f, kColTextWhite);

	if (ImGui::BeginPopup(popupId)) {
		ImGui::TextDisabled("Select Color");
		ImGui::Separator();
		if (colorToEdit) {
			float col[4] = { colorToEdit->x, colorToEdit->y, colorToEdit->z, colorToEdit->w };
			if (ImGui::ColorPicker4("##picker", col, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs)) {
				*colorToEdit = ImVec4(col[0], col[1], col[2], col[3]);
			}
		}
		ImGui::EndPopup();
	}

	return pressed;
}

void BeginCard(const char* id, const char* title, void (*iconFunc)(ImDrawList*, ImVec2, float, ImU32), float w, float h) {
	ImGui::PushID(id);
	ImGui::BeginChild(id, ImVec2(w, h), false, ImGuiWindowFlags_NoScrollbar);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 pos = ImGui::GetWindowPos();
	const ImVec2 size = ImGui::GetWindowSize();

	// Card background & rounded border
	dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), kColCardBg, 6.0f);
	dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), kColCardBorder, 6.0f, 0, 1.0f);

	// Card Header: Icon + Title on left, `...` on right
	const float headerH = 34.0f;
	if (iconFunc) {
		iconFunc(dl, ImVec2(pos.x + 22.0f, pos.y + headerH * 0.5f), 15.0f, kColTextWhite);
	}

	dl->AddText(
		ImVec2(pos.x + 36.0f, pos.y + (headerH - ImGui::GetTextLineHeight()) * 0.5f),
		kColTextWhite, title
	);

	// Three dots options icon
	const ImVec2 dotsCenter(pos.x + size.x - 20.0f, pos.y + headerH * 0.5f);
	DrawMoreDotsIcon(dl, dotsCenter, 1.5f, kColTextMuted);

	// Cursor for card content
	ImGui::SetCursorPos(ImVec2(14.0f, headerH + 4.0f));
}

void EndCard() {
	ImGui::EndChild();
	ImGui::PopID();
}

} // namespace

void Menu::handleInput() {
	if (GetAsyncKeyState(VK_RSHIFT) & 1) {
		globals.settings.menuOpen = !globals.settings.menuOpen;
	}
}

void Menu::render() {
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	if (!globals.settings.menuOpen) {
		ImGui::GetBackgroundDrawList()->AddText(
			ImVec2(12.f, 12.f),
			IM_COL32(255, 255, 255, 220),
			"[RIGHT SHIFT] - Menu"
		);
		return;
	}

	// -------------------------------------------------------------------------
	// Main Window Configuration
	// -------------------------------------------------------------------------
	const ImVec2 windowSize(840.0f, 490.0f);
	ImGui::SetNextWindowPos(
		ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f),
		ImGuiCond_FirstUseEver
	);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, kColWindowBg);
	ImGui::PushStyleColor(ImGuiCol_Border, kColBorder);

	const ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar;

	if (!ImGui::Begin("MecchaChameleon_Root", &globals.settings.menuOpen, windowFlags)) {
		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
		return;
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 winPos = ImGui::GetWindowPos();
	const ImVec2 winSize = ImGui::GetWindowSize();

	// -------------------------------------------------------------------------
	// 1. TOP HEADER BAR
	// -------------------------------------------------------------------------
	const float headerHeight = 46.0f;
	dl->AddLine(
		ImVec2(winPos.x, winPos.y + headerHeight),
		ImVec2(winPos.x + winSize.x, winPos.y + headerHeight),
		kColHeaderBorder, 1.0f
	);

	// Left: Logo + App Title
	ID3D11ShaderResourceView* logoTex = globals.overlay ? globals.overlay->getLogoTexture() : nullptr;
	if (logoTex) {
		ImGui::SetCursorPos(ImVec2(16.0f, (headerHeight - 24.0f) * 0.5f));
		ImGui::Image((ImTextureID)logoTex, ImVec2(24.0f, 24.0f));
	} else {
		DrawLogo(dl, ImVec2(winPos.x + 28.0f, winPos.y + headerHeight * 0.5f), 18.0f, kColTextWhite);
	}
	dl->AddText(
		ImVec2(winPos.x + 48.0f, winPos.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f),
		kColTextWhite, "MecchaChameleon"
	);

	// Right: Status badge ("Status: Attached ●")
	const char* statusPrefix = "Status: ";
	const char* statusText = "Attached";
	const float statusX = winPos.x + winSize.x - 330.0f;
	const float statusY = winPos.y + (headerHeight - ImGui::GetTextLineHeight()) * 0.5f;

	dl->AddText(ImVec2(statusX, statusY), kColTextMuted, statusPrefix);
	const float prefixW = ImGui::CalcTextSize(statusPrefix).x;
	dl->AddText(ImVec2(statusX + prefixW, statusY), kColTextWhite, statusText);
	const float textW = ImGui::CalcTextSize(statusText).x;
	dl->AddCircleFilled(
		ImVec2(statusX + prefixW + textW + 8.0f, winPos.y + headerHeight * 0.5f),
		3.5f, kColIndicatorDot
	);

	// Right: Preset Dropdown ("Preset: Legit v")
	static int currentPreset = 0;
	static const char* presets[] = { "Legit", "Semi-Rage", "Rage", "Default" };
	ImGui::SetCursorPos(ImVec2(winSize.x - 170.0f, (headerHeight - 24.0f) * 0.5f));
	CompactDropdown("##top_preset", &currentPreset, presets, IM_ARRAYSIZE(presets), 110.0f);

	// Right: Close Button ('X')
	ImGui::SetCursorPos(ImVec2(winSize.x - 42.0f, (headerHeight - 24.0f) * 0.5f));
	if (ImGui::InvisibleButton("##close_btn", ImVec2(24.0f, 24.0f))) {
		globals.settings.menuOpen = false;
	}
	const bool closeHovered = ImGui::IsItemHovered();
	const ImU32 closeCol = closeHovered ? kColTextWhite : kColTextMuted;
	const ImVec2 closeCenter(winPos.x + winSize.x - 30.0f, winPos.y + headerHeight * 0.5f);
	dl->AddLine(ImVec2(closeCenter.x - 4.5f, closeCenter.y - 4.5f), ImVec2(closeCenter.x + 4.5f, closeCenter.y + 4.5f), closeCol, 1.6f);
	dl->AddLine(ImVec2(closeCenter.x + 4.5f, closeCenter.y - 4.5f), ImVec2(closeCenter.x - 4.5f, closeCenter.y + 4.5f), closeCol, 1.6f);

	// -------------------------------------------------------------------------
	// 2. LEFT SIDEBAR NAVIGATION
	// -------------------------------------------------------------------------
	static int activeTab = 1; // 0 = Combat, 1 = Visuals (Active in mockup), 2 = Settings
	const float sidebarWidth = 165.0f;
	const float bodyTop = headerHeight + 12.0f;
	const float footerHeight = 36.0f;
	const float bodyHeight = winSize.y - bodyTop - footerHeight;

	// Sidebar label
	dl->AddText(
		ImVec2(winPos.x + 18.0f, winPos.y + bodyTop + 4.0f),
		kColTextMuted, "NAVIGATION"
	);

	struct NavItem {
		const char* name;
		void (*icon)(ImDrawList*, ImVec2, float, ImU32);
	};
	static const NavItem navItems[] = {
		{ "Combat",   DrawCrosshairIcon },
		{ "Visuals",  DrawEyeIcon },
		{ "Settings", DrawGearIcon }
	};

	float navY = bodyTop + 24.0f;
	for (int i = 0; i < 3; ++i) {
		const bool isSelected = (activeTab == i);
		ImGui::SetCursorPos(ImVec2(10.0f, navY));

		char btnId[32];
		snprintf(btnId, sizeof(btnId), "##nav_%d", i);
		if (ImGui::InvisibleButton(btnId, ImVec2(sidebarWidth - 16.0f, 38.0f))) {
			activeTab = i;
		}
		const bool hovered = ImGui::IsItemHovered();

		const ImVec2 itemMin(winPos.x + 10.0f, winPos.y + navY);
		const ImVec2 itemMax(winPos.x + sidebarWidth - 6.0f, winPos.y + navY + 38.0f);

		if (isSelected) {
			dl->AddRectFilled(itemMin, itemMax, kColNavActiveBg, 5.0f);
			dl->AddRect(itemMin, itemMax, kColCardBorder, 5.0f, 0, 1.0f);
			// Left white active indicator pill
			dl->AddRectFilled(
				ImVec2(itemMin.x, itemMin.y + 7.0f),
				ImVec2(itemMin.x + 3.0f, itemMax.y - 7.0f),
				kColTextWhite, 2.0f
			);
		} else if (hovered) {
			dl->AddRectFilled(itemMin, itemMax, kColNavHoverBg, 5.0f);
		}

		const ImU32 itemTextCol = isSelected ? kColTextWhite : (hovered ? kColTextWhite : kColTextGray);
		navItems[i].icon(dl, ImVec2(itemMin.x + 22.0f, itemMin.y + 19.0f), 14.0f, itemTextCol);
		dl->AddText(
			ImVec2(itemMin.x + 42.0f, itemMin.y + (38.0f - ImGui::GetTextLineHeight()) * 0.5f),
			itemTextCol, navItems[i].name
		);

		navY += 42.0f;
	}

	// -------------------------------------------------------------------------
	// 3. MAIN CONTENT (CARDS GRID)
	// -------------------------------------------------------------------------
	const float contentX = sidebarWidth + 10.0f;
	const float contentW = winSize.x - contentX - 16.0f;

	// Section subtitle
	dl->AddText(
		ImVec2(winPos.x + contentX, winPos.y + bodyTop + 4.0f),
		kColTextMuted, "MAIN CONTENT (Cards / Group Containers)"
	);

	ImGui::SetCursorPos(ImVec2(contentX, bodyTop + 24.0f));
	ImGui::BeginChild("##cards_container", ImVec2(contentW, bodyHeight - 20.0f), false, ImGuiWindowFlags_NoScrollbar);

	const float cardGap = 12.0f;
	const float cardW = (contentW - cardGap) * 0.5f;

	// Team options for ESP inline selectors
	static const char* teamOptions[] = { "Enemy", "Team" };
	static int teamSelection[3] = { 0, 0, 0 }; // Box, Skeleton, Name

	if (activeTab == 1) { // VISUALS TAB (Exact match to image!)
		const float topCardH = 175.0f;
		const float btmCardH = 170.0f;

		// --- CARD 1: Player ESP ---
		ImGui::SetCursorPos(ImVec2(0.f, 0.f));
		BeginCard("card_player_esp", "Player ESP", DrawUserIcon, cardW, topCardH);
		{
			// Row 1: 2D Box
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("2D Box", &globals.settings.esp.box);
			ImGui::SameLine(cardW - 120.0f);
			CompactDropdown("##box_team", &teamSelection[0], teamOptions, 2, 72.0f);
			ImGui::SameLine(cardW - 40.0f);
			PaletteIconButton("##box_pal", teamSelection[0] == 0 ? &globals.settings.esp.enemyBoxColor : &globals.settings.esp.defaultBoxColor);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			// Row 2: Skeleton
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Skeleton", &globals.settings.esp.skeleton);
			ImGui::SameLine(cardW - 120.0f);
			CompactDropdown("##skel_team", &teamSelection[1], teamOptions, 2, 72.0f);
			ImGui::SameLine(cardW - 40.0f);
			PaletteIconButton("##skel_pal", teamSelection[1] == 0 ? &globals.settings.esp.enemySkeletonColor : &globals.settings.esp.defaultSkeletonColor);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			// Row 3: Name & Distance
			static bool nameAndDist = false;
			nameAndDist = globals.settings.esp.name && globals.settings.esp.distance;
			ImGui::SetCursorPosX(16.0f);
			if (CustomCheckbox("Name & Distance", &nameAndDist)) {
				globals.settings.esp.name = nameAndDist;
				globals.settings.esp.distance = nameAndDist;
			}
			ImGui::SameLine(cardW - 120.0f);
			CompactDropdown("##name_team", &teamSelection[2], teamOptions, 2, 72.0f);
			ImGui::SameLine(cardW - 40.0f);
			PaletteIconButton("##name_pal", teamSelection[2] == 0 ? &globals.settings.esp.enemyBoxColor : &globals.settings.esp.defaultBoxColor);
		}
		EndCard();

		// --- CARD 2: Render Styles ---
		ImGui::SetCursorPos(ImVec2(cardW + cardGap, 0.f));
		BeginCard("card_render_styles", "Render Styles", DrawCubeIcon, cardW, topCardH);
		{
			// Row 1: Dynamic Box Sizing
			ImGui::SetCursorPosX(16.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Dynamic Box Sizing");
			ImGui::SameLine(cardW - 38.0f);
			CustomCheckbox("##dyn_box", &globals.settings.esp.dynamicBoxes);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			// Row 2: Snaplines (Tracers)
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Snaplines (Tracers)", &globals.settings.esp.snaplines);
			ImGui::SameLine(cardW - 120.0f);
			static const char* snaplineOptions[] = { "Bot", "Center", "Top" };
			CompactDropdown("##snap_pos", &globals.settings.esp.snaplinesPos, snaplineOptions, 3, 72.0f);
			ImGui::SameLine(cardW - 40.0f);
			PaletteIconButton("##snap_pal", &globals.settings.esp.enemySnaplineColor);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

			// Row 3: Minimap Scale
			static float minimapScale = 1.2f;
			ImGui::SetCursorPosX(16.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Minimap Scale");
			ImGui::SameLine(cardW - 95.0f);
			ImGui::PushItemWidth(80.0f);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, kColControlBg);
			ImGui::PushStyleColor(ImGuiCol_Border, kColControlBorder);
			ImGui::DragFloat("##mm_scale", &minimapScale, 0.05f, 0.5f, 3.0f, "%.1f");
			ImGui::PopStyleColor(2);
			ImGui::PopItemWidth();
		}
		EndCard();

		// --- CARD 3: Filters & Teams ---
		ImGui::SetCursorPos(ImVec2(0.f, topCardH + cardGap));
		BeginCard("card_filters", "Filters & Teams", DrawFunnelIcon, cardW, btmCardH);
		{
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Only Enemies", &globals.settings.esp.onlyEnemies);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Minimap Radar", &globals.settings.esp.minimap);
		}
		EndCard();

		// --- CARD 4: FoV & Screen ---
		ImGui::SetCursorPos(ImVec2(cardW + cardGap, topCardH + cardGap));
		BeginCard("card_fov_screen", "FoV & Screen", DrawCrosshairIcon, cardW, btmCardH);
		{
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Draw FoV Circle", &globals.settings.esp.fovCircle);
			ImGui::SameLine(cardW - 40.0f);
			static ImVec4 fovColor = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
			PaletteIconButton("##fov_pal", &fovColor);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Chinese Hat", &globals.settings.esp.chineseHat);
		}
		EndCard();
	}
	else if (activeTab == 0) { // COMBAT TAB
		const float topCardH = 175.0f;
		const float btmCardH = 170.0f;

		// Card 1: Aimbot Core
		ImGui::SetCursorPos(ImVec2(0.f, 0.f));
		BeginCard("card_aim_core", "Aimbot Core", DrawCrosshairIcon, cardW, topCardH);
		{
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Enable Aimbot", &globals.settings.aimbot.enabled);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("FOV Limit", &globals.settings.aimbot.fovLimit);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Smooth Aiming", &globals.settings.aimbot.smoothing);
		}
		EndCard();

		// Card 2: Aim Parameters
		ImGui::SetCursorPos(ImVec2(cardW + cardGap, 0.f));
		BeginCard("card_aim_params", "Aim Tuning", DrawCubeIcon, cardW, topCardH);
		{
			ImGui::SetCursorPosX(16.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("FOV Radius");
			ImGui::SameLine(cardW - 130.0f);
			ImGui::PushItemWidth(115.0f);
			ImGui::SliderFloat("##aim_fov", &globals.settings.aimbot.fov, 10.0f, 360.0f, "%.0f px");
			ImGui::PopItemWidth();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Smooth Factor");
			ImGui::SameLine(cardW - 130.0f);
			ImGui::PushItemWidth(115.0f);
			ImGui::SliderFloat("##aim_smooth", &globals.settings.aimbot.smooth, 1.0f, 25.0f, "%.1f");
			ImGui::PopItemWidth();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Target Key");
			ImGui::SameLine(cardW - 95.0f);
			static const char* keyOptions[] = { "RMB", "LMB", "ALT", "SHIFT" };
			static int selectedKey = 0;
			CompactDropdown("##key_drop", &selectedKey, keyOptions, 4, 80.0f);
		}
		EndCard();

		// Card 3: Hitboxes
		ImGui::SetCursorPos(ImVec2(0.f, topCardH + cardGap));
		BeginCard("card_hitboxes", "Hitbox Selection", DrawUserIcon, cardW * 2.0f + cardGap, btmCardH);
		{
			static bool hbHead = true, hbNeck = true, hbChest = false, hbPelvis = false;
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Target Head", &hbHead);
			ImGui::SameLine(180.0f);
			CustomCheckbox("Target Neck", &hbNeck);
			ImGui::SameLine(340.0f);
			CustomCheckbox("Target Chest", &hbChest);
			ImGui::SameLine(500.0f);
			CustomCheckbox("Target Pelvis", &hbPelvis);
		}
		EndCard();
	}
	else if (activeTab == 2) { // SETTINGS TAB
		ImGui::SetCursorPos(ImVec2(0.f, 0.f));
		BeginCard("card_app_settings", "Application Settings", DrawGearIcon, cardW, 240.0f);
		{
			ImGui::SetCursorPosX(16.0f);
			CustomCheckbox("Developer Mode", &globals.settings.esp.devMode);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
			ImGui::SetCursorPosX(16.0f);
			ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "Target: PenguinHotel-Win64-Shipping.exe");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
			ImGui::SetCursorPosX(16.0f);
			ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "Overlay: DXGI 11 Hardware Composited");
		}
		EndCard();

		ImGui::SetCursorPos(ImVec2(cardW + cardGap, 0.f));
		BeginCard("card_profiles", "Config Profiles", DrawCubeIcon, cardW, 240.0f);
		{
			ImGui::SetCursorPosX(16.0f);
			if (ImGui::Button("Save Current Config", ImVec2(160.0f, 28.0f))) {}
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			if (ImGui::Button("Load Config", ImVec2(160.0f, 28.0f))) {}
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
			ImGui::SetCursorPosX(16.0f);
			if (ImGui::Button("Reset to Defaults", ImVec2(160.0f, 28.0f))) {}
		}
		EndCard();
	}

	ImGui::EndChild();

	// -------------------------------------------------------------------------
	// 4. FOOTER BAR
	// -------------------------------------------------------------------------
	const float footerY = winSize.y - footerHeight;
	dl->AddLine(
		ImVec2(winPos.x, winPos.y + footerY),
		ImVec2(winPos.x + winSize.x, winPos.y + footerY),
		kColHeaderBorder, 1.0f
	);

	// Left: Keyboard icon + "Shortcut: [RIGHT SHIFT] to toggle"
	DrawKeyboardIcon(dl, ImVec2(winPos.x + 24.0f, winPos.y + footerY + footerHeight * 0.5f), 14.0f, kColTextMuted);
	dl->AddText(
		ImVec2(winPos.x + 38.0f, winPos.y + footerY + (footerHeight - ImGui::GetTextLineHeight()) * 0.5f),
		kColTextMuted, "Shortcut: [RIGHT SHIFT] to toggle"
	);

	// Right: "V3.6"  |  "FPS: 240.0"
	const char* verStr = "V3.6";
	const char* fpsStr = "FPS: 240.0";
	const float fpsW = ImGui::CalcTextSize(fpsStr).x;
	const float verW = ImGui::CalcTextSize(verStr).x;

	const float fpsX = winPos.x + winSize.x - fpsW - 20.0f;
	const float sepX = fpsX - 16.0f;
	const float verX = sepX - verW - 16.0f;
	const float footerTextY = winPos.y + footerY + (footerHeight - ImGui::GetTextLineHeight()) * 0.5f;

	dl->AddText(ImVec2(verX, footerTextY), kColTextMuted, verStr);
	dl->AddLine(
		ImVec2(sepX, winPos.y + footerY + 8.0f),
		ImVec2(sepX, winPos.y + footerY + footerHeight - 8.0f),
		kColCardBorder, 1.0f
	);
	dl->AddText(ImVec2(fpsX, footerTextY), kColTextMuted, fpsStr);

	ImGui::End();
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(3);
}
