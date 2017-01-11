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

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void drawCube()
{
	GLfloat vertices[] =
	{
		-1, -1, -1,   -1, -1,  1,   -1,  1,  1,   -1,  1, -1,
		 1, -1, -1,    1, -1,  1,    1,  1,  1,    1,  1, -1,
		-1, -1, -1,   -1, -1,  1,    1, -1,  1,    1, -1, -1,
		-1,  1, -1,   -1,  1,  1,    1,  1,  1,    1,  1, -1,
		-1, -1, -1,   -1,  1, -1,    1,  1, -1,    1, -1, -1,
		-1, -1,  1,   -1,  1,  1,    1,  1,  1,    1, -1,  1
	};

	GLfloat colors[] =
	{
		0, 0, 0,   0, 0, 1,   0, 1, 1,   0, 1, 0,
		1, 0, 0,   1, 0, 1,   1, 1, 1,   1, 1, 0,
		0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,
		0, 1, 0,   0, 1, 1,   1, 1, 1,   1, 1, 0,
		0, 0, 0,   0, 1, 0,   1, 1, 0,   1, 0, 0,
		0, 0, 1,   0, 1, 1,   1, 1, 1,   1, 0, 1
	};

	static float alpha = 0;
	//attempt to rotate cube
	glRotatef(alpha, 0, 1, 0);

	/* We have a color array and a vertex array */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(3, GL_FLOAT, 0, colors);

	/* Send data : 24 vertices */
	glDrawArrays(GL_QUADS, 0, 24);

	/* Cleanup states */
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	alpha += 1;
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



	// Load Fonts
	// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImColor(114, 144, 154);

	static bool show_app_log = true;

	const char* areas[4] = { "Chest_Left", "Chest_Right", "Upper_Ab_Left", "Upper_Ab_Right" };
	const char* ids[4] = { "0x01", "0x23", "0x42", "0x12" };
	const int statuses[4] = { 1,1,0, 1 };
	// Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
       
		ImGui_ImplGlfwGL3_NewFrame();

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			#pragma region Status Window
			ImGui::Begin("Status");
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
				ImGui::BeginChild("EngineStatus", ImVec2(0, 50), true);
				ImGui::Text("Engine status");
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Online");
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
				ImGui::BeginChild("SuitStatus", ImVec2(0, 50), true);
				ImGui::Text("Suit status");
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unplugged");
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}
			ImGui::End();
			#pragma endregion

			#pragma region Log Window
			if (show_app_log) { ShowLog(&show_app_log); }
			#pragma endregion

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
					ImGui::Text("Test all motors at full power"); ImGui::NewLine();
					ImGui::Button("Start"); ImGui::SameLine();
					ImGui::Button("Stop");
					ImGui::EndChild();
					ImGui::PopStyleVar();
				}
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
				}
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
					ImGui::BeginChild("PadByPad", ImVec2(0, 120), true);
					ImGui::Text("Test pad by pad using "); ImGui::SameLine();
					static int selected_sequence = 0;
					const char* names[] = { "Click", "Buzz", "Hum" };

					if (ImGui::Button("Sequence.."))
						ImGui::OpenPopup("select");
					ImGui::SameLine();
					ImGui::Text(names[selected_sequence]);
					if (ImGui::BeginPopup("select"))
					{
						for (int i = 0; i < 3; i++)
							if (ImGui::Selectable(names[i]))
								selected_sequence = i;
						ImGui::EndPopup();
					}
					ImGui::NewLine();
					ImGui::Columns(4);
					const char* pads[8] = { "BL-2", "CL-1", "BX-4", "LF-3", "LF-4", "LF-4", "NA-3" , "NA-4"};
					for (int i = 0; i < 8; i++)
					{
						if (i % 2 == 0 && i != 0) {
							ImGui::NextColumn();
						}
						
						ImGui::Button(pads[i], ImVec2(-1.0f, 0.0f));
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
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				
			}
			ImGui::End();
			#pragma endregion
        }

		
	
		
		// Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		


		//custom code
		static const GLfloat g_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			0.0f,  1.0f, 0.0f,
		};
		// This will identify our vertex buffer
		GLuint vertexbuffer;
		// Generate 1 buffer, put the resulting identifier in vertexbuffer
		glGenBuffers(1, &vertexbuffer);
		// The following commands will talk about our 'vertexbuffer' buffer
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// Give our vertices to OpenGL.
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		//end custom code
		glUseProgram(0);
		 ImGui::Render();


        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
