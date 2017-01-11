#pragma once
#include "imgui.h"
struct Log {
	ImGuiTextBuffer Buf;
	ImGuiTextFilter Filter;
	ImVector<int> LineOffsets;
	bool ScrollToBottom;
	void Clear();
	void AddLog(const char* fmt, ...);
	void Draw(const char* title, bool* p_open = NULL);
};

void ShowLog(bool* p_open);