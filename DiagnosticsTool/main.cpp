// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include "stdafx.h"
#include "GL\glew.h"

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
#include <iostream>
#include "AreaFlags.h"

using namespace nsvr;
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}



void buzz(NSVR_System* system, uint32_t area) {

	using namespace std::chrono;



	NSVR_EventList* events = NSVR_EventList_Create();
	NSVR_Event* basicEvent = NSVR_Event_Create(NSVR_EventType::BASIC_HAPTIC_EVENT);

	//One call in wrapper
	NSVR_Event_SetFloat(basicEvent, "duration", 0.3f);
	NSVR_Event_SetFloat(basicEvent, "strength", 1.0f);
	NSVR_Event_SetInteger(basicEvent, "area", area);
	NSVR_Event_SetInteger(basicEvent, "effect", NSVR_Effect::Bump); //doom_buzz
	NSVR_Event_SetFloat(basicEvent, "time", 0.0f);

	NSVR_EventList_AddEvent(events, basicEvent);

	uint32_t handle = NSVR_System_GenerateHandle(system);


	NSVR_EventList_Transmit(system, events, handle);


	NSVR_Event_Release(basicEvent);
	NSVR_EventList_Release(events);

	NSVR_System_DoHandleCommand(system, handle, NSVR_HandleCommand::PLAY);


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
		std::fill(values, values + NUM_SAMPLES, 0);
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

int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//if we use 3.3, we need to use shaders to render anything..
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	const char* areas[4] = { "Chest_Left", "Chest_Right", "Upper_Ab_Left", "Upper_Ab_Right" };
	const char* ids[4] = { "0x01", "0x23", "0x42", "0x12" };
	const int statuses[4] = { 1,1,0, 1 };
	// Main loop
	QuaternionDisplay display_chest;
	QuaternionDisplay display_leftUpperArm;
	QuaternionDisplay display_rightUpperArm;


	std::map<std::string, AreaFlag> padToAreaFlag = {
		{ "B2L", AreaFlag::Back_Left },

		{ "A1L", AreaFlag::Shoulder_Left },
		{ "A2L", AreaFlag::Upper_Arm_Left },
		{ "A3L", AreaFlag::Forearm_Left },
		{ "A1R", AreaFlag::Shoulder_Right } ,
		{ "A2R", AreaFlag::Upper_Arm_Right },
		{ "A3R", AreaFlag::Forearm_Right },
		{ "C1R", AreaFlag::Chest_Right },
		{ "C2R", AreaFlag::Upper_Ab_Right },
		{ "C3R", AreaFlag::Mid_Ab_Right },
		{ "C4R", AreaFlag::Lower_Ab_Right },
		{ "B2R", AreaFlag::Back_Right },

		{ "C1L", AreaFlag::Chest_Left },
		{ "C2L", AreaFlag::Upper_Ab_Left },
		{ "C3L", AreaFlag::Mid_Ab_Left },
		{ "C4L", AreaFlag::Lower_Ab_Left }
	


	};
	

	
	//Let's build the test effect. We'll iterate through all the pads, and then create an event for each.
	
	std::vector<std::string> pads = { "B2L", "A1L", "C1L", "C2L", "C1R", "C2R", "B2R", "A1R" , "A2L", "A3L", "C3L", "C4L", "C3R", "C4R", "A2R", "A3R"};

	//Instantiate the plugin
	NSVR_System* system = NSVR_System_Create();
	if (!system) {
		std::cout << "Failed to instantiate the NSVR Plugin";
		abort();
	}

	using namespace std::chrono;
	auto now = steady_clock::now();

	//Create the list to hold our events
	NSVR_EventList* events = NSVR_EventList_Create();
	float offset = 0.0f;
	for (const auto& area : pads) {
		NSVR_Event* myEvent = NSVR_Event_Create(NSVR_EventType::BASIC_HAPTIC_EVENT);

		NSVR_Event_SetFloat(myEvent, "duration", 0.7f);
		NSVR_Event_SetFloat(myEvent, "strength", 1.0f);
		NSVR_Event_SetInteger(myEvent, "area",(int) padToAreaFlag[area]);
		NSVR_Event_SetInteger(myEvent, "effect", 666); //doom_buzz
		NSVR_Event_SetFloat(myEvent, "time", offset);
		NSVR_EventList_AddEvent(events, myEvent);
		offset += 1.0f;
	}

	unsigned int test_effect_handle = NSVR_System_GenerateHandle(system);
	NSVR_EventList_Transmit(system, events, test_effect_handle);
	auto newTime = duration_cast<microseconds>(steady_clock::now() - now);
	std::cout << "Time (micro): " << newTime.count() << '\n';

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
       
		ImGui_ImplGlfwGL3_NewFrame();

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			#pragma region Status Window

			NSVR_System_Status status = { 0 };
			NSVR_System_PollStatus(system, &status);


			ImGui::Begin("Status");
			{
				
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
				ImGui::BeginChild("EngineStatus", ImVec2(0, 50), true);
				ImGui::Text("Service connection status");
				if (status.ConnectedToService) {
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");

				}
				else {
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");

				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
				
			}
			{
				if (status.ConnectedToService) {
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("SuitStatus", ImVec2(0, 50), true);
					ImGui::Text("Suit status");
					if (status.ConnectedToSuit) {
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Plugged in");
					}
					else {
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unplugged");
					}
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}
			
			}
			ImGui::End();
			#pragma endregion

			#pragma region Log Window
			if (show_app_log) { ShowLog(&show_app_log); }
			#pragma endregion
			ImGui::ShowMetricsWindow();
			#pragma region Motor Diagnostics
			ImGui::Begin("Motors");
			{
				ImGui::Columns(4, "mycolumns4", false);
				ImGui::Separator();
				ImGui::Text("ID"); ImGui::NextColumn();
				ImGui::Text("Name"); ImGui::NextColumn();
				ImGui::Text("Status"); ImGui::NextColumn();
				ImGui::Text("Operation"); ImGui::NextColumn();
				ImGui::Separator();
				for (int i = 0; i < 4; i++) {
					//ImGui::NextColumn();
					ImGui::Text(ids[i]); ImGui::NextColumn();
					ImGui::Text(areas[i]); ImGui::NextColumn();
					if (statuses[i]) {
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Nominal");
						ImGui::NextColumn();
						ImGui::Button("Get Info"); ImGui::NextColumn();
					}
					else {
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Broken");
						ImGui::NextColumn();
						ImGui::Button("Diagnose"); ImGui::NextColumn();
					} 
				

				}
				ImGui::Columns(1);
				ImGui::Separator();

			}
			ImGui::End();
			#pragma endregion

			#pragma region Motor Tests
			ImGui::Begin("Tests");
			{
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("FullPower", ImVec2(0, 70), true);
					ImGui::Text("Test all pads sequentially"); ImGui::NewLine();
					if (ImGui::Button("Go")) {
						NSVR_System_DoHandleCommand(system, test_effect_handle, NSVR_HandleCommand::PLAY);

						//testSeq.Play(AreaFlag::All_Areas);
						//plugin.Play(allPadsTest);
					}
					ImGui::SameLine();
					if (ImGui::Button("Stop")) {
						NSVR_System_DoHandleCommand(system, test_effect_handle, NSVR_HandleCommand::RESET);


						//plugin.Stop(allPadsTest);
					}
				
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}
				/*
				{
					static int glueBusterTimeDelay = 30;
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("GlueBuster", ImVec2(0, 120), true);
					ImGui::Text("Bust glue with this great test"); ImGui::NewLine();
					ImGui::Text("Run every");
					ImGui::SameLine();
					ImGui::InputInt("minutes", &glueBusterTimeDelay, 1, 5, 0);
					if (glueBusterTimeDelay <= 1) {
						glueBusterTimeDelay = 1;
					}
					ImGui::NewLine();
					ImGui::Button("Start"); ImGui::SameLine();
					ImGui::Button("Stop");
					ImGui::SameLine();
				
				
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}*/
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("PadByPad", ImVec2(0, 150), true);
					ImGui::Text("Test pad by pad ");
					
					ImGui::NewLine();
					ImGui::Columns(4);
					
					for (int i = 0; i < pads.size(); i++)
					{
						if (i % 2 == 0 && i != 0) {
							ImGui::NextColumn();
						}
						
						if (ImGui::Button(pads[i].c_str(), ImVec2(-1.0f, 0.0f))) {

							using namespace std::chrono;
							auto now = steady_clock::now();
							for (int x = 0; x < 100; x++) {
								buzz(system, (uint32_t)padToAreaFlag[pads[i].c_str()]);
							}
							auto newTime = duration_cast<microseconds>(steady_clock::now() - now);
							std::cout << "Time (micro): " << newTime.count() << '\n';
							
						}
					}


					ImGui::EndChild();

					ImGui::PopStyleVar();
					

				}
			}
			ImGui::End();
			#pragma endregion

			#pragma region Tracking View 
			ImGui::Begin("Tracking");
			{
				/*if (ImGui::Button("Enable tracking")) {
					plugin.SetTrackingEnabled(true);
				}
				ImGui::SameLine();
				if (ImGui::Button("Disable tracking")) {
					plugin.SetTrackingEnabled(false);
				}*/
				ImGui::NewLine();
				/*auto tracking = plugin.PollTracking();
				if (plugin.IsValidQuaternion(tracking.chest)) {
					ImGui::Text("Chest IMU");
					display_chest.Update(tracking.chest.x, tracking.chest.y, tracking.chest.z, tracking.chest.w);
					display_chest.Render();
				}
				if (plugin.IsValidQuaternion(tracking.right_upper_arm)) {
					ImGui::Text("Right Upper Arm IMU");

					display_rightUpperArm.Update(tracking.right_upper_arm.x, tracking.right_upper_arm.y, tracking.right_upper_arm.z, tracking.right_upper_arm.w);
					display_rightUpperArm.Render();
				}
				if (plugin.IsValidQuaternion(tracking.left_upper_arm)) {
					ImGui::Text("Left Upper Arm IMU");

					display_leftUpperArm.Update(tracking.left_upper_arm.x, tracking.left_upper_arm.y, tracking.left_upper_arm.z, tracking.left_upper_arm.w);
					display_leftUpperArm.Render();
				}*/
			
		
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

	NSVR_System_Release(system);

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}

