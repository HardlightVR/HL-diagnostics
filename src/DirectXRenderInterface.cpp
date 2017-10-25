#include "stdafx.h"
#include "DirectXRenderInterface.h"
#include <cassert>
#include <vector>
DirectXRenderInterface::DirectXRenderInterface(ImDrawList * drawList) : m_list(drawList)
{
}


ImColor toImColor(const dd::DrawVertex& v) {
	return ImColor(v.line.r, v.line.g, v.line.b);
}

ImVec2 toImVec2(const dd::DrawVertex& v) {
	return ImVec2(v.line.x, v.line.y);
}
void DirectXRenderInterface::drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled)
{
	assert(lines != nullptr);
	assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);


	for (int i = 0; i < count - 1; i++) {
		auto p1 = toImVec2(lines[i]);
		auto p2 = toImVec2(lines[i + 1]);

		p1.x *= 10;
		p1.y *= 10;
		p2.x *= 10;
		p2.y *= 10;
		p1.x += m_origin.x;
		p1.y += m_origin.y;
		p2.x += m_origin.x;
		p2.y += m_origin.y;
		m_list->AddLine(p1, p2, toImColor(lines[i]));

	}



}

void DirectXRenderInterface::SetDrawlist(ImDrawList * drawList)
{
	m_list = drawList;
}

void DirectXRenderInterface::SetOrigin(ImVec2 origin) {
	m_origin = origin;
}