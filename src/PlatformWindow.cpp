#include "stdafx.h"
#include "PlatformWindow.h"
#include "imgui.h"
#include <vector>
#include <sstream>
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

static const char* const device_concepts[] = { 
	"Unknown"
	"Suit",
	"Controller",
	"Headwear",
	"Gun",
	"Sword" 
};

static const char* const node_concepts[] = {
	"Unknown"
	"Haptic",
	"LED",
	"InertialTracker",
	"AbsoluteTracker"
};

PlatformWindow::PlatformWindow(hvr_platform * platform, NSVR_System* plugin)
	: m_platform(platform)
	, m_plugin(plugin)
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
	renderPlatformSide();
	renderPluginSide();

}

void renderPlatformVersion()
{
	static uint32_t version = hvr_platform_getversion();

	std::stringstream s;
	s << "Hardlight Platform " << (version >> 16) << "." << ((version << 16) >> 16);
	ImGui::Text(s.str().c_str());
}

void PlatformWindow::renderAvailableHardwarePlugins()
{
	ImGui::Begin("Plugin Diagnostics");
	renderPlatformVersion();

	ImGui::Text("Loaded plugins:");
	hvr_plugin_list plugins;
	hvr_platform_enumerateplugins(m_platform, &plugins);

	for (int i = 0; i < plugins.count; i++) {
		hvr_plugin_info info = { 0 };
		hvr_platform_getplugininfo(m_platform, plugins.ids[i], &info);
		if (ImGui::CollapsingHeader(info.name))
		{
			hvr_platform_updatediagnostics(m_platform, plugins.ids[i]);
		}
	}
	
	ImGui::End();
}

void PlatformWindow::renderPlatformSide()
{
	renderPlatformUI();
	renderAvailableHardwarePlugins();
}


void PlatformWindow::renderPluginSide()
{

	NSVR_DeviceInfo_Iter iter;
	NSVR_DeviceInfo_Iter_Init(&iter);
	while (NSVR_DeviceInfo_Iter_Next(&iter, m_plugin)) {

		ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
		ImGui::BeginChild(iter.DeviceInfo.Name, ImVec2(0, 100), true, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text(iter.DeviceInfo.Name);
			ImGui::Text("Status: "); ImGui::SameLine();
			if (iter.DeviceInfo.Status == NSVR_DeviceStatus_Connected) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.2f, 1.0f), "connected");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "disconnected");

			}
			ImGui::Text("ID: %d", iter.DeviceInfo.Id);
			ImGui::Text("Concept: %s", device_concepts[iter.DeviceInfo.Concept]);
			if (ImGui::CollapsingHeader("Nodes")) {
				NSVR_NodeInfo_Iter node;
				NSVR_NodeInfo_Iter_Init(&node);
				while (NSVR_NodeInfo_Iter_Next(&node, iter.DeviceInfo.Id, m_plugin)) {
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 3.0f);
					ImGui::BeginChild(node.NodeInfo.Name, ImVec2(0, 30), true);

					//TODO(fix the node concept enum in the plugin to match the hardware)
					ImGui::Text("%s [%s]", node.NodeInfo.Name, node_concepts[node.NodeInfo.Type]);
					ImGui::EndChild();

					ImGui::PopStyleVar();
				}
			}
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
}

void PlatformWindow::renderPlatformUI()
{
	ImGui::Begin("Device Manager");
	{
		if (ImGui::Button("Create virtual Hardlight Suit")) {
			static uint32_t device_id = 20;
			hvr_platform_createdevice(m_platform, device_id);
			device_id++;
		}
	}
	ImGui::End();
}
