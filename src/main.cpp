// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include "stdafx.h"
#include "GL\glew.h"
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <vector>
#include <GLFW/glfw3.h>
#include "LogWindow.h"
#include "ShaderOps.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <unordered_map>
#include <map>
#include <chrono>

#include "NSLoader.h"
#include "NSLoader_Internal.h"
#include <iostream>
#include "AreaFlags.h"
#include <thread>
#include <sstream>

#include "JsonKeyValueConfig.h"
using namespace nsvr;

unsigned char hex2byte(char digit)
{
	char hexDigit = toupper(digit);
	return hexDigit >= 'A' ? hexDigit - 'A' + 10 : hexDigit - '0';
}

const char* CONCEPT_TO_STRING[] = { "Unknown", "Suit", "Controller", "Headware", "Gun", "Sword" };


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

bool isValidQuaternion(NSVR_Quaternion& q) {
	return !(std::abs(q.x) < 0.001 
		&& std::abs(q.y) < 0.001
		&& std::abs(q.z) < 0.001
		&& std::abs(q.w) < 0.001);
}


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

NSVR_Event* createEvent(float time, float duration, float strength, uint32_t area, int effect) {
	NSVR_Event* event = nullptr;
	NSVR_Event_Create(&event, NSVR_EventType::NSVR_EventType_BasicHapticEvent);
	NSVR_Event_SetFloat(event, "duration",duration);
	NSVR_Event_SetFloat(event, "strength", strength);
	NSVR_Event_SetUInt32s(event, "area", &area, 1);
	NSVR_Event_SetInt(event, "effect", effect); 
	NSVR_Event_SetFloat(event, "time", time);
	return event;
}

NSVR_PlaybackHandle* createHandle(NSVR_Timeline* timeline, NSVR_System* system) {
	NSVR_PlaybackHandle* handle = nullptr;
	NSVR_PlaybackHandle_Create(&handle);
	NSVR_Result result = NSVR_Timeline_Transmit(timeline,system, handle);
	//NSVR_Result result = NSVR_PlaybackHandle_Bind(handle, timeline);
	
	if (NSVR_FAILURE(result)) {
		std::cout << "result: " << result;
	}
	return handle;
}
void makeBuzz(NSVR_System* system, uint32_t area) {

	//create a timeline
	NSVR_Timeline* timeline = nullptr;
	NSVR_Timeline_Create(&timeline);
	//add a buzz to the timeline
	NSVR_Event* buzz = createEvent(0.0f, 0.3f, 1.0f, area, 666);
	NSVR_Timeline_AddEvent(timeline, buzz);

	//create a handle from the timeline
	NSVR_PlaybackHandle* handle = createHandle(timeline, system);

	//play & release
	NSVR_PlaybackHandle_Command(handle, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play);
	NSVR_PlaybackHandle_Release(&handle);
	NSVR_Timeline_Release(&timeline);

}
class ValueGraph {
public:
	void Render() {
		for (int i = 0; i < NUM_SAMPLES-1; i++) {
			if (i == 0) {
				continue;
			}
			values[i] = values[i + 1];

		}
	
		ImGui::PlotLines(_name.c_str(), values, NUM_SAMPLES, 0, 0, -1, 1.0, ImVec2(300, 40));
	}
	ValueGraph(std::string name) :_name(name) {};
	~ValueGraph() {
		std::fill(values, values + NUM_SAMPLES, 0.0f);
	}
	void AddValue(float f) {
		
		values[NUM_SAMPLES-1] = f;
	}
private:
	const static int NUM_SAMPLES = 2000;
	std::string _name;
	float values[NUM_SAMPLES];

};
class QuaternionDisplay {
public:
	void Render() {
		_xGraph.Render();
		_yGraph.Render();
		_zGraph.Render();
		_wGraph.Render();
	}
	QuaternionDisplay() :_xGraph("X"), _yGraph("Y"), _zGraph("Z"), _wGraph("W") {}
	~QuaternionDisplay() {}
	void Update(float x, float y, float z, float w) {
		_xGraph.AddValue(x);
		_yGraph.AddValue(y);
		_zGraph.AddValue(z);
		_wGraph.AddValue(w);
	}
private:
	ValueGraph _xGraph;
	ValueGraph _yGraph;
	ValueGraph _zGraph;
	ValueGraph _wGraph;
};


inline uint8_t AsciiHexToByte(char digit1, char digit2)
{
	return hex2byte(digit1) * 16 + hex2byte(digit2);
}

#define DRV_STATUS_NOMINAL 1
#define DRV_STATUS_UNKNOWN 0
#define DRV_STATUS_OVERCURRENT 2
#define DRV_STATUS_OVERTEMP 3
#define DRV_STATUS_OVERCURRENT_OVERTEMP 4

void processDrvDiagnostics(std::string m,  const std::unordered_map<std::string, std::string>& zones, std::unordered_map<std::string, int>& statuses)
{
	int drv_id = 0;
	int over_current = 0;
	int over_temp = 0;
	sscanf_s(m.c_str(), "[DriverMain] DRVDIAG %d,%d,%d",  &drv_id, &over_current, &over_temp);

	for (const auto& pair : zones) {
		if (AsciiHexToByte(pair.second[2], pair.second[3]) == drv_id) {
			if (over_current && over_temp) {
				statuses[pair.first] = DRV_STATUS_OVERCURRENT_OVERTEMP;
			}
			else if (over_current) {
				statuses[pair.first] = DRV_STATUS_OVERCURRENT;
			}
			else if (over_temp) {
				statuses[pair.first] = DRV_STATUS_OVERTEMP;
			}
			else {
				statuses[pair.first] = DRV_STATUS_NOMINAL;
			}
		}
	}

}

int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//if we use 3.3, we need to use shaders to render anything..
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Hardlight Diagnostics", NULL, NULL);
	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
	}
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);

	GLuint programID = LoadShaders("VertexShader.txt", "FragmentShader.txt");

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	static bool show_app_log = true;
	static Log log;

	const char* areas[16] = { "Forearm Left", "Forearm Right","Upper Arm Left", "Upper Arm Right",  "Shoulder Left", "Shoulder Right", "Upper Back Left", "Upper Back Right",  "Chest Left", "Chest Right", "Upper Ab Left", "Upper Ab Right", "Mid Ab Left", "Mid Ab Right", "Lower Ab Left", "Lower Ab Right"};
	//const char* ids[16] =   { "0x10"        , "0x18",         "0x16",           "0x1E",             "0x17",           "0x1F",          "0x11",            "0x19",               "0x15",      "0x1D",        "0x12",         "0x1A",            "0x13",       "0x1B",          "0x12",          "0x1A" };
	int statuses[16] = { DRV_STATUS_UNKNOWN };

	auto parseHexValue = [](const Json::Value& val) {
		const char* hexChars = &val.asCString()[2];
		return AsciiHexToByte(hexChars[0], hexChars[1]);
	};

	auto zones = nsvr::tools::json::parseDictFromDict<std::string, std::string>("Zones.json", [](auto val) { return val.asString(); }, [](auto val) {return val.asString(); });
	std::unordered_map<std::string, int> status_map;
	for (const auto& kvp : zones) {
		status_map[kvp.first] = DRV_STATUS_UNKNOWN;
	}

	auto padToAreaFlag = nsvr::tools::json::parseDictFromDict<std::string, AreaFlag>("PadToZone.json", [](auto val) {return val.asString(); }, [](auto val) {return AreaFlag(val.asInt()); });
	// Main loop
	QuaternionDisplay display_chest;
	QuaternionDisplay display_leftUpperArm;
	QuaternionDisplay display_rightUpperArm;

	bool _suitConnected = false;
	
	

	
	//Let's build the test effect. We'll iterate through all the pads, and then create an event for each.
	
	std::vector<std::string> pads = { "B2R", "A1R", "C1R", "C2R", "C1L", "C2L", "B2L", "A1L" , "A2R", "A3R", "C3R", "C4R", "C3L", "C4L", "A2L", "A3L"};

	std::vector<uint32_t> order = {
		nsvr_region_lower_arm_left,
		nsvr_region_upper_arm_left,
		nsvr_region_shoulder_left,
		nsvr_region_upper_back_left,

		nsvr_region_chest_left,
		nsvr_region_upper_ab_left,
		nsvr_region_middle_ab_left,
		nsvr_region_lower_ab_left,

		nsvr_region_lower_ab_right,
		nsvr_region_middle_ab_right,
		nsvr_region_upper_ab_right,
		nsvr_region_chest_right,

		nsvr_region_upper_back_right,
		nsvr_region_shoulder_right,
		nsvr_region_upper_arm_right,
		nsvr_region_lower_arm_right

	};
	//Instantiate the plugin
	NSVR_System* system = nullptr;
	if (NSVR_System_Create(&system) != NSVR_Success_Unqualified) {
		std::cout << "Failed to instantiate the NSVR Plugin";
		abort();
	}

	using namespace std::chrono;


	

	NSVR_Timeline* padByPad = nullptr;
	NSVR_Timeline_Create(&padByPad);
	float offset = 0.0f;
	for (const auto& area : order) {
		NSVR_Event* myEvent = createEvent(offset, 0.7f, 1.0f, (int)area, 3);
		NSVR_Timeline_AddEvent(padByPad, myEvent);
		NSVR_Event_Release(&myEvent);
		offset += 1.0f;
	}

	NSVR_PlaybackHandle* padByPadHandle = createHandle(padByPad, system);




    while (!glfwWindowShouldClose(window))
    {
// #pragma region Window placement
// 		ImGui::SetWindowPos("Status", ImVec2(15, 10));
// 		ImGui::SetWindowSize("Status", ImVec2(241, 140));
// 
// 		ImGui::SetWindowPos("Stats", ImVec2(12, 159));
// 		ImGui::SetWindowSize("Stats", ImVec2(243, 194));
// 
// 		ImGui::SetWindowPos("Modes", ImVec2(11, 361));
// 		ImGui::SetWindowSize("Modes", ImVec2(246, 228));
// 
// 		ImGui::SetWindowPos("Tests", ImVec2(266, 10));
// 		ImGui::SetWindowSize("Tests", ImVec2(457, 366));
// 
// 		ImGui::SetWindowPos("Communication", ImVec2(268, 387));
// 		ImGui::SetWindowSize("Communication", ImVec2(455, 229));
// 
// 		ImGui::SetWindowPos("Tracking", ImVec2(733, 10));
// 		ImGui::SetWindowSize("Tracking", ImVec2(493, 19));
// 
// 		ImGui::SetWindowPos("Motors", ImVec2(735, 42));
// 		ImGui::SetWindowSize("Motors", ImVec2(466, 381));
// 
// 		ImGui::SetWindowPos("Hex", ImVec2(734, 430));
// 		ImGui::SetWindowSize("Hex", ImVec2(248, 126));
// #pragma endregion


		using namespace std::chrono;
	
        glfwPollEvents();
       
		ImGui_ImplGlfwGL3_NewFrame();

     
		{
			#pragma region Status Window

			
	
			bool serviceConnected = NSVR_SUCCESS(NSVR_System_GetServiceInfo(system, nullptr));
			NSVR_Result deviceConnected = false;
			ImGui::Begin("Status");
			{
				
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("EngineStatus", ImVec2(0, 50), true);
						ImGui::Text("Service connection status"); ImGui::SameLine();
						ShowHelpMarker("The NullSpace VR Runtime Service must be running to use the suit. Check for the small NullSpace icon in the task bar, and make sure that it is green. If it is not green, right click it and hit 'Enable Suit'. ");
						if (serviceConnected) {
							ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
						}
						else {
							ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");

						}
					ImGui::EndChild();
				ImGui::PopStyleVar();
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("KnownDevices", ImVec2(0, 150), true);
						ImGui::Text("Known Devices");
						if (serviceConnected) {
							NSVR_DeviceInfo_Iter iter;
							NSVR_DeviceInfo_Iter_Init(&iter);
	
							while (NSVR_DeviceInfo_Iter_Next(&iter, system)) {
								ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
								if (iter.DeviceInfo.Status == NSVR_DeviceStatus_Connected) {
									ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
								}
								else {
									ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

								}
								ImGui::BeginChild("KnownDevices", ImVec2(0, 42), true, ImGuiWindowFlags_ShowBorders);
								ImGui::Text(iter.DeviceInfo.Name); ImGui::SameLine();
								
								std::string id("[id: " + std::to_string(iter.DeviceInfo.Id) + "]");
								ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6, 1.0f), id.c_str());
							
								ImGui::TextColored(ImVec4(0.8f, 0.0f, 1.0f, 1.0f), CONCEPT_TO_STRING[iter.DeviceInfo.Concept]);
								ImGui::EndChild();
								ImGui::PopStyleColor();
								ImGui::PopStyleVar();
							}
						}
						else {
							ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Waiting for service..");
						}
					ImGui::EndChild();
				ImGui::PopStyleVar();
				
			}
			
			ImGui::End();
			#pragma endregion

			#pragma region Log Window
			NSVR_LogEntry entry = { 0 };
			if (NSVR_System_PollLogs(system, &entry) == NSVR_Success_Unqualified) {
				std::string m(entry.Message);
				if (m.find("DRVDIAG") != std::string::npos) {
					processDrvDiagnostics(m, zones, status_map);
				}
				else {
					m.append("\n");
					log.AddLog(m.c_str());
				}
			}
			if (show_app_log) { ShowLog(log, &show_app_log); }
			#pragma endregion
			ImGui::ShowMetricsWindow();


			#pragma region Motor Diagnostics
			ImGui::Begin("Motors");
			{
				ImGui::Columns(3, "mycolumns4", false);
				ImGui::Separator();
				ImGui::Text("ID"); ImGui::NextColumn();
				ImGui::Text("Name"); ImGui::NextColumn();
				ImGui::Text("Status"); ImGui::NextColumn();
			//	ImGui::Text("Operation"); ImGui::NextColumn();
				ImGui::Separator();
				for (int i = 0; i < 16; i++) {
					std::string areaReadableName = areas[i];
					std::string areaKey = areaReadableName;
					std::replace(areaKey.begin(), areaKey.end(), ' ', '_');

					ImGui::Text(areaKey.c_str()); ImGui::NextColumn();
					ImGui::Text(areaReadableName.c_str()); ImGui::NextColumn();

					int status = status_map[areaKey];
					switch (status) {
					case DRV_STATUS_NOMINAL:
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Nominal");
						ImGui::SameLine();
						ShowHelpMarker("The motor is functioning normally");
						break;

					case  DRV_STATUS_UNKNOWN: 
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Unknown");
						ImGui::SameLine();
						ShowHelpMarker("Refresh the diagnostics");
						break;

					case DRV_STATUS_OVERCURRENT:
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Over current");
						ImGui::SameLine();
						ShowHelpMarker("Motor stall: may be stuck or damaged");
						break;

					case DRV_STATUS_OVERTEMP:
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Over temp");
						ImGui::SameLine();
						ShowHelpMarker("Motor overheated: may be due to excessive runtime");
						break;

					case DRV_STATUS_OVERCURRENT_OVERTEMP:
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Over temp & current");
						ImGui::SameLine();
						ShowHelpMarker("Dual stall and overheat: is the suit on fire?");
						break;

					default:
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Fatal error");
						ImGui::SameLine();
						ShowHelpMarker("This should not show up");
						break;
					}

					ImGui::NextColumn();
				}
				

				
				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::PushTextWrapPos(350.0f);
				ImGui::Text("If any of the motors have failures, please email lucian@nullspacevr.com immediately");
				ImGui::PopTextWrapPos();
				if (ImGui::Button("Refresh Diagnostics")) {
					NSVR_System_DumpDeviceDiagnostics(system);
				}
			}
			ImGui::End();
			#pragma endregion

			#pragma region Motor Tests
			ImGui::Begin("Tests");
			{
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("Warning", ImVec2(0, 30), true);

					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning!"); ImGui::SameLine();
					ImGui::Text("Do not run multiple tests at the same time");
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}

				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("Sequential", ImVec2(0, 70), true);
					ImGui::Text("Test all pads sequentially"); ImGui::SameLine();
					ShowHelpMarker("The suit will play a strong buzz on each pad, beginning on the left forearm and traversing up to the back, then down to the bottom left ab, then up to the right back, and finally down to the right forearm.");
					ImGui::NewLine();
					if (ImGui::Button("Start")) {
						NSVR_PlaybackHandle_Command(padByPadHandle, NSVR_PlaybackCommand_Reset);
						NSVR_PlaybackHandle_Command(padByPadHandle, NSVR_PlaybackCommand_Play);
					}
					ImGui::SameLine();
					if (ImGui::Button("Stop")) {
						NSVR_PlaybackHandle_Command(padByPadHandle, NSVR_PlaybackCommand_Reset);
					}
				
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("FullPower", ImVec2(0, 70), true);
					ImGui::Text("Activate entire suit at specified power."); ImGui::NewLine();
					static float f1 = 0.0f;
					static bool _isEffectPlaying;
					bool wasSliderMoved = ImGui::SliderFloat("", &f1, 0.0f, 1.0f, "strength = %.3f");

					std::vector<double> strengths(16, f1);

					uint32_t* areas = (uint32_t*)order.data();
					if (wasSliderMoved) {
						NSVR_Immediate_Set(system, areas, strengths.data(), strengths.size());
					}
					
					if (ImGui::Button("Start")) {
						_isEffectPlaying = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Stop")) {
						_isEffectPlaying = false;
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();
				}
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("PadByPad", ImVec2(0, 150), true);
					ImGui::Text("Test pad by pad "); ImGui::SameLine();
					ShowHelpMarker("Plays a strong buzz on the selected pad. Buttons are arranged as a bird's eye view of the suit.");
					ImGui::NewLine();
					ImGui::Columns(4);
					
					for (std::size_t i = 0; i < pads.size(); i++)
					{
						if (i % 2 == 0 && i != 0) {
							ImGui::NextColumn();
						}
						
						if (ImGui::Button(pads[i].c_str(), ImVec2(-1.0f, 0.0f))) {
							uint32_t area = (uint32_t)padToAreaFlag[pads[i].c_str()];
							makeBuzz(system, area);
						}
					}


					ImGui::EndChild();

					ImGui::PopStyleVar();
					

				}
			}
			ImGui::End();
			#pragma endregion



#pragma region Modes
		

			ImGui::Begin("Modes");
			{

				ImGui::Text("Set mode of the suit");
				if (ImGui::Button("Enable audio mode")) {
					NSVR_System_Audio_Enable(system, nullptr);
				}
				if (ImGui::Button("Disable audio mode")) {
					NSVR_System_Audio_Disable(system);

				}
				static int audio_min = 0x04;
				static int audio_max = 0x22;
				static int filter = 0x01;
				static int peak_time = 0x01;

				bool changed = false;
				changed |= ImGui::SliderInt("Audio max", &audio_max, 0, 255);
				changed |= ImGui::SliderInt("Audio min", &audio_min, 0, 255);
				changed |= ImGui::SliderInt("Peak time", &peak_time, 0, 3);
				changed |= ImGui::SliderInt("Filter", &filter, 0,3);

				if (changed) {
					NSVR_AudioOptions options = { 0 };
					options.AudioMax = audio_max;
					options.AudioMin = audio_min;
					options.Filter = filter;
					options.PeakTime = peak_time;

					NSVR_System_Audio_Enable(system, &options);
				}



			}
			ImGui::End();
#pragma endregion

#pragma region Hex input


			ImGui::Begin("Hex Console");
			{
	
				static char header[16] = {'2', '4', '0', '2'};
				static char footer[16] = { 'F', 'F', 'F', 'F', '0', 'A' };
				static char buf[64]; 
				memset(buf, 0, 64);
				static bool auto_headerfooter = true;


				if (ImGui::InputText("", buf, 64, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue)) {
					int actual_len = 0;

					uint8_t intbuf[32] = { 0 };
					
					if (auto_headerfooter) {
						int header_length = 0;
						int footer_length = 0;
						for (int i = 0; i < 16; i++) {
							char headerChar = header[i];
							if (headerChar == '\0') {
								header_length = i;
								break;
							}
						}
						for (int i = 0; i < 16; i++) {
							char footerChar = footer[i];
							if (footerChar == '\0') {
								footer_length = i;
								break;
							}
						}

						for (int i = 0; i < header_length; i+=2) {
							intbuf[i / 2] = AsciiHexToByte(header[i], header[i + 1]);
						}
						
						int header_byte_length = header_length / 2;
						int footer_byte_length = footer_length / 2;
						for (int i = 0; i < 64; i += 2) {
							if (buf[i] == '\0') {

								actual_len = int(i / 2);
								break;
							}
							//stepping two at a time. We take two chars = 2 hex digits = 1 byte
							char digit1 = buf[i];
							char digit2 = buf[i + 1];

							uint8_t byte = AsciiHexToByte(digit1, digit2);
							std::cout << "Byte " << i << ": " << int(byte) << '\n';
							intbuf[int(i / 2) + header_byte_length] = byte;

						}

						for (int i = 0; i < footer_length; i += 2) {
							intbuf[(i / 2) + actual_len + header_byte_length] = AsciiHexToByte(footer[i], footer[i + 1]);
						}
					
						actual_len += header_byte_length + footer_byte_length;

					}
					else {


						for (int i = 0; i < 64; i += 2) {
							if (buf[i] == '\0') {

								actual_len = int(i / 2);
								break;
							}
							//stepping two at a time. We take two chars = 2 hex digits = 1 byte
							char digit1 = buf[i];
							char digit2 = buf[i + 1];

							uint8_t byte = AsciiHexToByte(digit1, digit2);
							std::cout << "Byte " << i << ": " << int(byte) << '\n';
							intbuf[int(i / 2)] = byte;

						}
					}
					
					NSVR_System_SubmitRawCommand(system, intbuf,actual_len);
					memset(buf, 0, 64);

				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetKeyboardFocusHere(-1);
				}


				
				ImGui::PushItemWidth(60);
				ImGui::InputText("Header", header, 16, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
				ImGui::InputText("Footer", footer, 16, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

				ImGui::Checkbox("Use header and footer", &auto_headerfooter);

				ImGui::PopItemWidth();
			}
			ImGui::End();
#pragma endregion
			#pragma region Tracking View 
			ImGui::Begin("Tracking");
			{
				if (ImGui::Button("Enable tracking")) {
					NSVR_System_Tracking_Enable(system);
				}
				ImGui::SameLine();
				if (ImGui::Button("Disable tracking")) {
					NSVR_System_Tracking_Disable(system);
				}
				ImGui::NewLine();

				NSVR_TrackingUpdate tracking = { 0 };
				NSVR_System_Tracking_Poll(system, &tracking);

				if (isValidQuaternion(tracking.chest)) {
					ImGui::Text("Chest IMU");
					display_chest.Update(tracking.chest.x, tracking.chest.y, tracking.chest.z, tracking.chest.w);
					display_chest.Render();
				}
				if (isValidQuaternion(tracking.right_upper_arm)) {
					ImGui::Text("Right Upper Arm IMU");

					display_rightUpperArm.Update(tracking.right_upper_arm.x, tracking.right_upper_arm.y, tracking.right_upper_arm.z, tracking.right_upper_arm.w);
					display_rightUpperArm.Render();
				}
				if (isValidQuaternion(tracking.left_upper_arm)) {
					ImGui::Text("Left Upper Arm IMU");

					display_leftUpperArm.Update(tracking.left_upper_arm.x, tracking.left_upper_arm.y, tracking.left_upper_arm.z, tracking.left_upper_arm.w);
					display_leftUpperArm.Render();
				}
			
		
			}
			ImGui::End();
			#pragma endregion
        }

		
	
		
		// Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
      

		//end custom code
		glUseProgram(0);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		 ImGui::Render();
		


        glfwSwapBuffers(window);

		
    }
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	NSVR_System_Release(&system);

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}

