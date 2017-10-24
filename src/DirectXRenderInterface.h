#pragma once
#include "debug_draw.hpp"
#include "imgui.h"
class DirectXRenderInterface final : public dd::RenderInterface
{
public:
	DirectXRenderInterface(ImDrawList* drawList);

//	virtual void drawPointList(const DrawVertex * points, int count, bool depthEnabled);
	virtual void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled) override;
	void SetDrawlist(ImDrawList* drawList);

	void SetOrigin(ImVec2 origin);
private:
	ImDrawList* m_list;
	ImVec2 m_origin;
};