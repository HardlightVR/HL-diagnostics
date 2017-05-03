#include "stdafx.h"
#include "LogWindow.h"
void Log::Clear()
{
	Buf.clear();
	LineOffsets.clear();
}

void Log::AddLog(const char * fmt, ...)
{
	int oldSize = Buf.size();
	va_list args;
	va_start(args, fmt);
	Buf.appendv(fmt, args);
	va_end(args);
	for (int newSize = Buf.size(); oldSize < newSize; oldSize++) {
		if (Buf[oldSize] == '\n')
			LineOffsets.push_back(oldSize);
	}
	ScrollToBottom = true;
}

void Log::Draw(const char * title, bool * p_open)
{
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(title, p_open);
	if (ImGui::Button("Clear")) { Clear(); }
	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	Filter.Draw("Filter", -100.0f);
	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (copy) { ImGui::LogToClipboard(); }
	
	if (Filter.IsActive()) {
		const char* bufBegin = Buf.begin();
		const char* line = bufBegin;
		for (int lineNo = 0; line != NULL; lineNo++) {
			const char* line_end = (lineNo < LineOffsets.Size) ? bufBegin + LineOffsets[lineNo] : NULL;
			if (Filter.PassFilter(line, line_end))
				ImGui::TextUnformatted(line, line_end);
			line = line_end && line_end[1] ? line_end + 1 : NULL;
		
		}
	}
	else {
		ImGui::TextUnformatted(Buf.begin());

	}
	if (ScrollToBottom) {
		ImGui::SetScrollHere(1.0f);
	}
	ScrollToBottom = false;
	ImGui::EndChild();
	ImGui::End();
}

void ShowLog(Log& log, bool * p_open)
{
	static float last_time = -1.0f;
	float time = ImGui::GetTime();
	//if (time - last_time >= 0.3f)
	//{
	//	const char* random_words[] = { "system", "info", "warning", "error", "fatal", "notice", "log" };
	//	log.AddLog("[engine] engine rec'd 3 effects\n");
	//	last_time = time;
	//}

	log.Draw("Communications", p_open);
}
