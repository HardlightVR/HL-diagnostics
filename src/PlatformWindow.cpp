#include "stdafx.h"
#include "PlatformWindow.h"
#include "imgui.h"
#include <vector>
#include "HLVR_Errors.h"
#include <sstream>
#include <unordered_map>
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
	"Unknown",
	"Suit",
	"Controller",
	"Headwear",
	"Gun",
	"Sword" 
};

static const char* const node_concepts[] = {
	"Unknown",
	"Haptic",
	"LED",
	"InertialTracker",
	"AbsoluteTracker"
};

static const std::unordered_map<uint32_t, const char*> region_names = {
	{ 0,  "unknown" },
	{ 1 * HLVR_SUBREGION_BLOCK , "body" },
	{ 2 * HLVR_SUBREGION_BLOCK , "torso" },
	{ 3 * HLVR_SUBREGION_BLOCK , "torso_front" },
	{ 4 * HLVR_SUBREGION_BLOCK , "chest_left" },
	{ 5 * HLVR_SUBREGION_BLOCK , "chest_right" },
	{ 6 * HLVR_SUBREGION_BLOCK , "upper_ab_left" },
	{ 7 * HLVR_SUBREGION_BLOCK , "middle_ab_left" },
	{ 8 * HLVR_SUBREGION_BLOCK , "lower_ab_left" },
	{ 9 * HLVR_SUBREGION_BLOCK , "upper_ab_right" },
	{ 10 * HLVR_SUBREGION_BLOCK , "middle_ab_right" },
	{ 11 * HLVR_SUBREGION_BLOCK , "lower_ab_right" },
	{ 12 * HLVR_SUBREGION_BLOCK , "torso_back" },
	{ 13 * HLVR_SUBREGION_BLOCK , "torso_left" },
	{ 14 * HLVR_SUBREGION_BLOCK , "torso_right" },
	{ 15 * HLVR_SUBREGION_BLOCK , "upper_back_left" },
	{ 16 * HLVR_SUBREGION_BLOCK , "upper_back_right" },
	{ 17 * HLVR_SUBREGION_BLOCK , "upper_arm_left" },
	{ 18 * HLVR_SUBREGION_BLOCK , "lower_arm_left" },
	{ 19 * HLVR_SUBREGION_BLOCK , "upper_arm_right" },
	{ 20 * HLVR_SUBREGION_BLOCK , "lower_arm_right" },
	{ 21 * HLVR_SUBREGION_BLOCK , "shoulder_left" },
	{ 22 * HLVR_SUBREGION_BLOCK , "shoulder_right" },
	{ 23 * HLVR_SUBREGION_BLOCK , "upper_leg_left" },
	{ 24 * HLVR_SUBREGION_BLOCK , "lower_leg_left" },
	{ 25 * HLVR_SUBREGION_BLOCK , "upper_leg_right" },
	{ 26 * HLVR_SUBREGION_BLOCK , "lower_leg_right" },
	{ 27 * HLVR_SUBREGION_BLOCK , "head" },
	{ 28 * HLVR_SUBREGION_BLOCK , "palm_left" },
	{ 29 * HLVR_SUBREGION_BLOCK , "palm_right" },
};

PlatformWindow::PlatformWindow(hvr_platform * platform, HLVR_Agent* plugin)
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

	
	HLVR_BodyView_Create(&m_retainedBodyview);
}

void PlatformWindow::Render()
{

	renderEmulation();
	renderPlatformSide();
	renderPluginSide();

}

PlatformWindow::~PlatformWindow()
{
	HLVR_BodyView_Release(&m_retainedBodyview);
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

	HLVR_DeviceIterator iter;
	HLVR_DeviceIterator_Init(&iter);
	while (HLVR_OK(HLVR_DeviceIterator_Next(&iter, m_plugin))) {

		ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
		ImGui::BeginChild(iter.DeviceInfo.Name, ImVec2(0, 100), true, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text(iter.DeviceInfo.Name);
			ImGui::Text("Status: "); ImGui::SameLine();
			if (iter.DeviceInfo.Status == HLVR_DeviceStatus_Connected) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.2f, 1.0f), "connected");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "disconnected");

			}
			ImGui::Text("ID: %d", iter.DeviceInfo.Id);
			ImGui::Text("Concept: %s", device_concepts[iter.DeviceInfo.Concept]);
			if (ImGui::CollapsingHeader("Nodes")) {
				HLVR_NodeIterator node;
				HLVR_NodeIterator_Init(&node);
				while (HLVR_OK(HLVR_NodeIterator_Next(&node, iter.DeviceInfo.Id, m_plugin))) {
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 3.0f);
					ImGui::BeginChild(node.NodeInfo.Name, ImVec2(0, 30), true);

					//TODO(fix the node concept enum in the plugin to match the hardware)
					ImGui::Text("%s [%s]", node.NodeInfo.Name, node_concepts[node.NodeInfo.Concept]);
					ImGui::EndChild();

					ImGui::PopStyleVar();
				}
			}
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
}

void PlatformWindow::renderEmulation()
{

	HLVR_BodyView_Poll(m_retainedBodyview, m_plugin);

	uint32_t numNodes = 0;
	HLVR_BodyView_GetNodeCount(m_retainedBodyview, &numNodes);

	ImGui::Begin("Body view");

	ImGui::Text("Displays activity on regions registered by the hardware devices."); ImGui::SameLine();
	ShowHelpMarker("The view is populated lazily when you begin to interact or send commands to the platform. Don't worry if it's blank");

	for (auto i = 0; i < numNodes; i++) {
		static float f;
		

		uint32_t region = hlvr_region_UNKNOWN;
		HLVR_BodyView_GetNodeRegion(m_retainedBodyview, i, &region);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(i / 7.0f, 0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(i / 7.0f, 0.9f, 0.9f));
		float intensity = 0;
		HLVR_BodyView_GetIntensity(m_retainedBodyview, i, &intensity);
		ImGui::VSliderFloat(std::string("##"+std::string(region_names.at(region))).c_str(), ImVec2(25, 40), &intensity, 0, 1.0, "%.1f");

		//ImGui::VSliderFloat(region_names.at(region), ImVec2(25, 40), &intensity, 0, 1.0, "%.1f");
		ImGui::PopStyleColor(4);

		if ((i % 8) < 7) ImGui::SameLine();
	}
	ImGui::End();



	/*static bool selected[16] = { true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true };
	for (int i = 0; i < 16; i++)
	{
		ImGui::PushID(i);
		if (ImGui::Selectable("Sailor", &selected[i], 0, ImVec2(50, 50)))
		{
			int x = i % 4, y = i / 4;
			if (x > 0) selected[i - 1] ^= 1;
			if (x < 3) selected[i + 1] ^= 1;
			if (y > 0) selected[i - 4] ^= 1;
			if (y < 3) selected[i + 4] ^= 1;
		}
		if ((i % 4) < 3) ImGui::SameLine();
		ImGui::PopID();
	}*/
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
