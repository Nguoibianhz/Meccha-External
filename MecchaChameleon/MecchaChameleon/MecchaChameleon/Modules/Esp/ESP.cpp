#include "ESP.hpp"
#include "../../Engine/ImGui/imgui.h"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <utility>

bool isRenderValid(bool sameTeam, bool onlyEnemiesEnabled, bool isLocalPlayer, bool devMode) {
	if (sameTeam && onlyEnemiesEnabled) return false;

	if (devMode) return true;

	if (isLocalPlayer) return false;

	return true;
}

// also very scuffed but its on my todo list
ImU32 getESPColor(const TrackedActor& actor, const ImVec4& defaultSetting, const ImVec4& enemySetting) {
	const ImVec4& color = (globals.settings.esp.isTeammateColorEnabled && !actor.sameTeam)
		? enemySetting
		: defaultSetting;
	return Custom::ColorToU32(color);
}

void drawOutlinedLine(ImDrawList* drawList, ImVec2 from, ImVec2 to, ImU32 color) {
	drawList->AddLine(from, to, IM_COL32(0, 0, 0, 255), 4.0f);
	drawList->AddLine(from, to, color, 2.0f);
}

void ESP::renderESP(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo) {
	if (globals.settings.esp.box)
		this->renderBox(actors, viewInfo);

	if (globals.settings.esp.corners)
		this->renderCorners(actors, viewInfo);

	if (globals.settings.esp.skeleton)
		this->renderSkeleton(actors, viewInfo);
	
	if (globals.settings.esp.name && globals.settings.esp.distance)
		this->renderNameDistance(actors, viewInfo, LabelType::NAMEDISTANCE);
	else if (globals.settings.esp.name)
		this->renderNameDistance(actors, viewInfo, LabelType::NAME);
	else if (globals.settings.esp.distance)
		this->renderNameDistance(actors, viewInfo, LabelType::DISTANCE);

	if (globals.settings.esp.snaplines)
		this->renderSnaplines(actors, viewInfo);

	if (globals.settings.esp.fovCircle)
		this->renderFoV(globals.settings.aimbot.fov);

	if (globals.settings.esp.minimap)
		this->renderMinimap(actors, viewInfo);
}

void ESP::renderBox(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	for (const TrackedActor& actor : actors) {
		if (!isRenderValid(actor.sameTeam, globals.settings.esp.onlyEnemies, actor.isLocalPlayer, globals.settings.esp.devMode)) continue;

		ActorScreenBounds bounds{};
		if (!tryGetActorScreenBoundsForEsp(
			this->unreal, actor, viewInfo, displaySize.x, displaySize.y,
			globals.settings.esp.dynamicBoxes, bounds
		))
			continue;

		const ImVec2 topLeft(bounds.left, bounds.top);
		const ImVec2 bottomRight(bounds.right, bounds.bottom);

		drawList->AddRect(
			topLeft,
			bottomRight,
			IM_COL32(0, 0, 0, 255),
			0.0f, ImDrawFlags_None, 4.0f
		);

		drawList->AddRect(
			topLeft,
			bottomRight,
			getESPColor(actor, globals.settings.esp.defaultBoxColor, globals.settings.esp.enemyBoxColor),
			0.0f, ImDrawFlags_None, 2.0f
		);
	}
}

void ESP::renderCorners(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	for (const TrackedActor& actor : actors) {
		if (!isRenderValid(actor.sameTeam, globals.settings.esp.onlyEnemies, actor.isLocalPlayer, globals.settings.esp.devMode)) continue;

		ActorScreenBounds bounds{};
		if (!tryGetActorScreenBoundsForEsp(
			this->unreal, actor, viewInfo, displaySize.x, displaySize.y,
			globals.settings.esp.dynamicBoxes, bounds
		))
			continue;

		const float left = bounds.left;
		const float right = bounds.right;
		const float top = bounds.top;
		const float bottom = bounds.bottom;
		const float width2D = right - left;
		const float height2D = bottom - top;

		const float lineWidth = width2D * 0.25f;
		const float lineHeight = height2D * 0.20f;

		auto drawCorner = [&](float x, float y, float directionX, float directionY) {
			drawList->AddLine(
				ImVec2(x, y),
				ImVec2(x + (directionX * lineWidth), y),
				IM_COL32(0, 0, 0, 255),
				4.0f
			);
			drawList->AddLine(
				ImVec2(x, y),
				ImVec2(x, y + (directionY * lineHeight)),
				IM_COL32(0, 0, 0, 255),
				4.0f
			);

			drawList->AddLine(
				ImVec2(x, y),
				ImVec2(x + (directionX * lineWidth), y),
				getESPColor(actor, globals.settings.esp.defaultBoxColor, globals.settings.esp.enemyBoxColor),
				2.0f
			);
			drawList->AddLine(
				ImVec2(x, y),
				ImVec2(x, y + (directionY * lineHeight)),
				getESPColor(actor, globals.settings.esp.defaultBoxColor, globals.settings.esp.enemyBoxColor),
				2.0f
			);
		};

		drawCorner(left, top, 1.0f, 1.0f);
		drawCorner(right, top, -1.0f, 1.0f);
		drawCorner(left, bottom, 1.0f, -1.0f);
		drawCorner(right, bottom, -1.0f, -1.0f);
	}
}

void ESP::renderSkeleton(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	for (const TrackedActor& actor : actors) {
		if (!isRenderValid(actor.sameTeam, globals.settings.esp.onlyEnemies, actor.isLocalPlayer, globals.settings.esp.devMode)) continue;
		if (actor.boneList.size() < kSkeletonBoneCount) continue;

		auto getBoneScreen = [&](int index, FVector2D& out) -> bool {
			const FVectorD& bone = actor.boneList[index];

			return this->unreal.WorldToScreen(
				viewInfo,
				FVector{ bone.x, bone.y, bone.z },
				out,
				displaySize.x,
				displaySize.y
			);
		};

		for (const std::pair<int, int>& pair : skeletonPairs) {
			FVector2D from, to;

			if (!getBoneScreen(pair.first, from) || !getBoneScreen(pair.second, to)) continue;

			const float dx = to.x - from.x;
			const float dy = to.y - from.y;
			if ((dx * dx + dy * dy) < 4.0f) continue;

			drawList->AddLine(
				ImVec2(from.x, from.y),
				ImVec2(to.x, to.y),
				getESPColor(actor, globals.settings.esp.defaultSkeletonColor, globals.settings.esp.enemySkeletonColor),
				1.0f
			);
		}
	}
}

void ESP::renderNameDistance(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo, LabelType type) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	FVector2D screenTop;

	for (const TrackedActor& actor : actors) {
		if (!isRenderValid(actor.sameTeam, globals.settings.esp.onlyEnemies, actor.isLocalPlayer, globals.settings.esp.devMode)) continue;

		bool bVisibleTop = this->unreal.WorldToScreen(
			viewInfo,
			actor.location + FVector(0, 0, actor.playerSize),
			screenTop,
			displaySize.x,
			displaySize.y
		);

		if (!bVisibleTop) continue;

		std::string text;

		if (type == LabelType::NAME) {
			text = actor.playerName;
		}
		else if (type == LabelType::DISTANCE) {
			FVector distanceFrom = viewInfo.Location;

			for (const TrackedActor& localActor : actors) {
				if (localActor.isLocalPlayer) {
					distanceFrom = localActor.location;
					break;
				}
			}

			double dx = actor.location.x - distanceFrom.x;
			double dy = actor.location.y - distanceFrom.y;
			double dz = actor.location.z - distanceFrom.z;

			double distance = std::sqrt(dx * dx + dy * dy + dz * dz) / 100.0;

			text = std::to_string(static_cast<int>(distance)) + "m";
		}
		else if (type == LabelType::NAMEDISTANCE) {
			FVector distanceFrom = viewInfo.Location;

			for (const TrackedActor& localActor : actors) {
				if (localActor.isLocalPlayer) {
					distanceFrom = localActor.location;
					break;
				}
			}

			double dx = actor.location.x - distanceFrom.x;
			double dy = actor.location.y - distanceFrom.y;
			double dz = actor.location.z - distanceFrom.z;

			double distance = std::sqrt(dx * dx + dy * dy + dz * dz) / 100.0;

			text = actor.playerName + " / " + std::to_string(static_cast<int>(distance)) + "m";
		}

		if (text.empty()) continue;

		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		ImVec2 textPos = ImVec2(
			screenTop.x - (textSize.x * 0.5f),
			screenTop.y - textSize.y - 4.0f
		);

		drawList->AddText(ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());
		drawList->AddText(ImVec2(textPos.x - 1.0f, textPos.y - 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());
		drawList->AddText(ImVec2(textPos.x - 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());
		drawList->AddText(ImVec2(textPos.x + 1.0f, textPos.y - 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());

		drawList->AddText(
			textPos,
			IM_COL32(255, 255, 255, 255),
			text.c_str()
		);
	}
}

void ESP::renderSnaplines(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	
	ImVec2 screenStart = { displaySize.x * 0.5f, displaySize.y };
	if (globals.settings.esp.snaplinesPos == 1) {
		screenStart = { displaySize.x * 0.5f, displaySize.y * 0.5f }; // Crosshair / Center
	} else if (globals.settings.esp.snaplinesPos == 2) {
		screenStart = { displaySize.x * 0.5f, 0.0f }; // Top
	}

	if (actors.empty()) return;

	for (const TrackedActor& actor : actors) {
		if (!isRenderValid(actor.sameTeam, globals.settings.esp.onlyEnemies, actor.isLocalPlayer, globals.settings.esp.devMode)) continue;

		const float halfHeight = (actor.playerSize >= 15.0 && actor.playerSize <= 300.0) 
			? static_cast<float>(actor.playerSize) 
			: 88.0f;

		FVector2D screenPos;
		if (!this->unreal.WorldToScreen(viewInfo, FVector(actor.location.x, actor.location.y, actor.location.z - halfHeight), screenPos, displaySize.x, displaySize.y)) {
			continue;
		}

		if (screenPos.x < 0 || screenPos.y < 0 || screenPos.x > displaySize.x || screenPos.y > displaySize.y) {
			continue;
		}

		drawList->AddLine(
			screenStart,
			ImVec2((float)screenPos.x, (float)screenPos.y),
			IM_COL32(0, 0, 0, 255),
			4.0f
		);

		drawList->AddLine(
			screenStart,
			ImVec2((float)screenPos.x, (float)screenPos.y),
			getESPColor(actor, globals.settings.esp.defaultSnaplineColor, globals.settings.esp.enemySnaplineColor),
			2.0f
		);
	}
}

void ESP::renderFoV(float fov) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	const ImVec2 displayMiddle = ImVec2(displaySize.x / 2, displaySize.y / 2);

	drawList->AddCircle(displayMiddle, fov, ImColor(0, 0, 0, 255), 64, 3.0f);
	drawList->AddCircle(displayMiddle, fov, ImColor(255, 255, 255, 255), 64, 1.0f);
}

void ESP::renderMinimap(const std::vector<TrackedActor>& actors, const FMinimalViewInfo& viewInfo) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	const ImVec2 mapMin(
		displaySize.x - kMinimapPadding - kMinimapSize,
		kMinimapPadding
	);
	const ImVec2 mapMax(
		displaySize.x - kMinimapPadding,
		kMinimapPadding + kMinimapSize
	);
	const ImVec2 center(
		(mapMin.x + mapMax.x) * 0.5f,
		(mapMin.y + mapMax.y) * 0.5f
	);
	const float halfSize = kMinimapSize * 0.5f;
	const float scale = halfSize / kMinimapWorldRange;
	const float playerYawRadians = static_cast<float>(viewInfo.Rotation.Yaw * (kPi / 180.0));

	drawList->AddRectFilled(
		mapMin,
		mapMax,
		IM_COL32(64, 64, 64, 210),
		0.f
	);
	drawList->AddRect(
		mapMin,
		mapMax,
		IM_COL32(0, 0, 0, 255),
		0.f,
		ImDrawFlags_None,
		1.f
	);

	drawList->AddLine(
		ImVec2(mapMin.x, center.y),
		ImVec2(mapMax.x, center.y),
		IM_COL32(24, 24, 24, 255),
		1.f
	);
	drawList->AddLine(
		ImVec2(center.x, mapMin.y),
		ImVec2(center.x, mapMax.y),
		IM_COL32(24, 24, 24, 255),
		1.f
	);

	const float halfFovRadians = viewInfo.FOV * 0.5f * (kPi / 180.f);
	const float forwardAngle = -kPi * 0.5f;
	const ImVec2 fovLeft(
		center.x + std::cos(forwardAngle - halfFovRadians) * halfSize,
		center.y + std::sin(forwardAngle - halfFovRadians) * halfSize
	);
	const ImVec2 fovRight(
		center.x + std::cos(forwardAngle + halfFovRadians) * halfSize,
		center.y + std::sin(forwardAngle + halfFovRadians) * halfSize
	);

	drawList->AddLine(center, fovLeft, IM_COL32(24, 24, 24, 255), 1.f);
	drawList->AddLine(center, fovRight, IM_COL32(24, 24, 24, 255), 1.f);

	FVector localLocation = viewInfo.Location;
	tryGetLocalPlayer(actors, localLocation);

	for (const TrackedActor& actor : actors) {
		if (!shouldShowOnMinimap(actor)) continue;

		const double deltaX = actor.location.x - localLocation.x;
		const double deltaY = actor.location.y - localLocation.y;
		const float distance = std::sqrt(static_cast<float>(deltaX * deltaX + deltaY * deltaY));

		if (distance > kMinimapWorldRange) continue;

		const ImVec2 mapPos = worldDeltaToMinimap(deltaX, deltaY, playerYawRadians, center, scale);
		if (mapPos.x < mapMin.x || mapPos.x > mapMax.x || mapPos.y < mapMin.y || mapPos.y > mapMax.y) continue;

		const ImU32 actorColor = getMinimapActorColor(actor);
		drawList->AddCircleFilled(mapPos, 2.5f, actorColor);

		const float facingRadians = getActorFacingRadians(actor);
		drawDirectionArrow(
			drawList,
			mapPos,
			facingRadians,
			playerYawRadians,
			IM_COL32(255, 220, 0, 255)
		);
	}
}