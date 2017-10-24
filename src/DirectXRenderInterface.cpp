#include "stdafx.h"
#include "DirectXRenderInterface.h"
#include <cassert>
#include <vector>
DirectXRenderInterface::DirectXRenderInterface(ImDrawList * drawList) : m_list(drawList)
{
}

std::vector<ImVec2> fromDdVertex(const dd::DrawVertex* lines, int count, const ImVec2& origin) {
	std::vector<ImVec2> out;
	for (int i = 0; i < count; i++) {
		out.emplace_back(lines[i].line.x * 10 + origin.x, lines[i].line.y * 10 + origin.y);
	}
	return out;
} 
void DirectXRenderInterface::drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled)
{
	assert(lines != nullptr);
	assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);
	m_list->AddPolyline(fromDdVertex(lines, count, m_origin).data(), count, ImColor(255, 0, 0), false, 1.0f, true);

	//for (int i = 0; i < count; i++) {
//	}



}

void DirectXRenderInterface::SetDrawlist(ImDrawList * drawList)
{
	m_list = drawList;
}

void DirectXRenderInterface::SetOrigin(ImVec2 origin) {
	m_origin = origin;
}