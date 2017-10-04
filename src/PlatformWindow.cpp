#include "stdafx.h"
#include "PlatformWindow.h"
#include "imgui.h"

//
//hvr_diagnostics_ui menu;
//menu.keyval = [](const char* key, const char* val) {
//	ImGui::Text("%s: %s", key, val);
//};
//menu.button = [](const char* label) {
//	return ImGui::Button(label);
//};
static void ShowHelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

PlatformWindow::PlatformWindow(hvr_platform * platform)
	: m_platform(platform)
{
	hvr_diagnostics_ui menu;
	menu.keyval = [](const char* key, const char* val) {
		ImGui::Text("%s: %s", key, val);
	};
	menu.button = [](const char* label) {
		return ImGui::Button(label);
	};

	hvr_platform_setupdiagnostics(m_platform, &menu);
}

void PlatformWindow::Render()
{
	renderPlatformUI();
	renderPluginUI();

}

void PlatformWindow::renderPluginUI()
{
	
	hvr_plugin_list plugins;
	hvr_platform_enumerateplugins(m_platform, &plugins);

	for (int i = 0; i < plugins.count; i++) {
		hvr_plugin_info info = { 0 };
		hvr_platform_getplugininfo(m_platform, plugins.ids[i], &info);
		ImGui::Begin(info.name);
		{
			hvr_platform_updatediagnostics(m_platform, plugins.ids[i]);
		}
		ImGui::End();
	}
	
}

void PlatformWindow::renderPlatformUI()
{
	ImGui::Begin("Device Manager");
	{
		if (ImGui::Button("Create new device")) {
			static uint32_t device_id = 20;
			hvr_platform_createdevice(m_platform, device_id);
		}
	}
	ImGui::End();
}
