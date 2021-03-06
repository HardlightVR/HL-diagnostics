// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include "stdafx.h"

#include <algorithm>
#include <stdio.h>
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <vector>
#include <sstream>
#include <memory>
#include <functional>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "NSDriverApi.h"
#include "PlatformWindow.h"
#include "HLVR.h"
#include <thread>
#include "HLVR_Experimental.h"
#include <iostream>
#include "DirectXRenderInterface.h"

#include <chrono>
// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

void CreateRenderTarget()
{
	DXGI_SWAP_CHAIN_DESC sd;
	g_pSwapChain->GetDesc(&sd);

	// Create the render target
	ID3D11Texture2D* pBackBuffer;
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
	ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));
	render_target_view_desc.Format = sd.BufferDesc.Format;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &g_mainRenderTargetView);
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

HRESULT CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return E_FAIL;

	CreateRenderTarget();

	return S_OK;
}



void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplDX11_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX11_InvalidateDeviceObjects();
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
			ImGui_ImplDX11_CreateDeviceObjects();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



struct Effect {
	Effect() {
		HLVR_Effect_Create(&m_ptr);
	}
	~Effect() {
		HLVR_Effect_Destroy(&m_ptr);
	}
	void Play() {
		HLVR_Effect_Play(m_ptr);
	}
	HLVR_Effect* m_ptr;

	HLVR_Effect* raw() { return m_ptr; }
};


struct Timeline {
	Timeline() {
		HLVR_Timeline_Create(&m_ptr);
	}
	~Timeline() {
		HLVR_Timeline_Destroy(&m_ptr);
	}
	void AddEvent(double time, HLVR_Event* data) {
		HLVR_Timeline_AddEvent(m_ptr, time, data);
	}
	void Transmit(HLVR_System* system, Effect* effect) {
		HLVR_Timeline_Transmit(m_ptr, system, effect->raw());
	}
	HLVR_Timeline* m_ptr;
};

struct NodeIterator {
	HLVR_NodeIterator m_it;
	uint32_t device;
	HLVR_System* system;
	NodeIterator(uint32_t device_id, HLVR_System* system) :device(device_id), system(system){
		HLVR_NodeIterator_Init(&m_it);
	}

	bool Next() {
		return HLVR_OK(HLVR_NodeIterator_Next(&m_it, device, system));
	}

	const HLVR_NodeInfo& Value() {
		return m_it.NodeInfo;
	}
};
void ShowDriverInformation() {
	ImGui::Begin("Platform Info");
	{
		std::stringstream s;
		auto version = hvr_platform_getversion();
		s << "HardlightPlatform.dll version " << (version >> 16) << "." << ((version << 16) >> 16);
		ImGui::Text(s.str().c_str());

		auto pluginVersion = HLVR_Version_Get();
		std::stringstream s2;
		s2 << "Hardlight.dll version " << HLVR_VERSION_STRING;
		ImGui::Text(s2.str().c_str());



	}
	ImGui::End();
}

auto DiscreteHaptic_withregions(uint32_t repetitions, HLVR_Waveform waveform, const std::vector<uint32_t>& regions, float strength = 1.0f) {
	HLVR_Event* event;
	HLVR_Event_Create(&event, HLVR_EventType_DiscreteHaptic);
	HLVR_Event_SetUInt32s(event, HLVR_EventKey_Target_Regions_UInt32s, regions.data(), regions.size());
	HLVR_Event_SetInt(event, HLVR_EventKey_DiscreteHaptic_Waveform_Int, waveform);
	HLVR_Event_SetUInt32(event, HLVR_EventKey_DiscreteHaptic_Repetitions_UInt32, repetitions);
	HLVR_Event_SetFloat(event, HLVR_EventKey_DiscreteHaptic_Strength_Float, strength);

	std::unique_ptr<HLVR_Event, std::function<void(HLVR_Event*)>> ptr(event, [](HLVR_Event* e) { HLVR_Event_Destroy(&e); });
	return ptr;
}
auto DiscreteHaptic_withnodes(uint32_t repetitions, HLVR_Waveform waveform, const std::vector<uint32_t>& nodes, float strength = 1.0f) {
	HLVR_Event* event;
	HLVR_Event_Create(&event, HLVR_EventType_DiscreteHaptic);
	HLVR_Event_SetUInt32s(event, HLVR_EventKey_Target_Nodes_UInt32s, nodes.data(), nodes.size());
	HLVR_Event_SetInt(event, HLVR_EventKey_DiscreteHaptic_Waveform_Int, waveform);
	HLVR_Event_SetUInt32(event, HLVR_EventKey_DiscreteHaptic_Repetitions_UInt32, repetitions);
	HLVR_Event_SetFloat(event, HLVR_EventKey_DiscreteHaptic_Strength_Float, strength);

	std::unique_ptr<HLVR_Event, std::function<void(HLVR_Event*)>> ptr(event, [](HLVR_Event* e) { HLVR_Event_Destroy(&e); });
	return ptr;
}

auto BufferedHaptic_withregions(const std::vector<float>& samples, float desiredFrequency, const std::vector<uint32_t>& regions) {
	HLVR_Event* event;
	HLVR_Event_Create(&event, HLVR_EventType_BufferedHaptic);
	HLVR_Event_SetUInt32s(event, HLVR_EventKey_Target_Regions_UInt32s, regions.data(), regions.size());
	HLVR_Event_SetFloats(event, HLVR_EventKey_BufferedHaptic_Samples_Floats, samples.data(), samples.size());
	HLVR_Event_SetFloat(event, HLVR_EventKey_BufferedHaptic_Frequency_Float, desiredFrequency);

	std::unique_ptr<HLVR_Event, std::function<void(HLVR_Event*)>> ptr(event, [](HLVR_Event* e) { HLVR_Event_Destroy(&e); });
	return ptr;
}

auto EnableAudio(const std::vector<uint32_t>& regions) {
	HLVR_Event* event;
	HLVR_Event_Create(&event, HLVR_EventType_BeginAnalogAudio);
	HLVR_Event_SetUInt32s(event, HLVR_EventKey_Target_Regions_UInt32s, regions.data(), regions.size());

	std::unique_ptr<HLVR_Event, std::function<void(HLVR_Event*)>> ptr(event, [](HLVR_Event* e) { HLVR_Event_Destroy(&e); });
	return ptr;
}

auto DisableAudio(const std::vector<uint32_t>& regions) {
	HLVR_Event* event;
	HLVR_Event_Create(&event, HLVR_EventType_EndAnalogAudio);
	HLVR_Event_SetUInt32s(event, HLVR_EventKey_Target_Regions_UInt32s, regions.data(), regions.size());

	std::unique_ptr<HLVR_Event, std::function<void(HLVR_Event*)>> ptr(event, [](HLVR_Event* e) { HLVR_Event_Destroy(&e); });
	return ptr;
}
void testPads(HLVR_System* system) {

	Timeline timeline;

	NodeIterator nodeIter(hlvr_allnodes, system);

	float timeOffset = 0.0f;
	while (nodeIter.Next()) {
		auto event = DiscreteHaptic_withnodes(0, HLVR_Waveform_Hum, { nodeIter.Value().Id });

		HLVR_Event_ValidationResult result;
		HLVR_Event_Validate(event.get(), &result);
		
		assert(result.Count == 0);

		timeline.AddEvent(timeOffset, event.get());
		timeOffset += 2.0f;
	}

	Effect effect;
	timeline.Transmit(system, &effect);
	effect.Play();
}

void DrawBoxWidget(const HLVR_Quaternion& q, const ddVec3& boxCenter = { 0.0f, 0.0f, 0.0f }) {
	const ddVec3 boxColor = { 0.0f, 0.8f, 0.8f };
	dd::box(boxCenter, boxColor, 10.5f, 10.5f, 10.5f);
	dd::cross(boxCenter, 10.0f);
}
void DrawQuaternionWidget(const HLVR_Quaternion& q, const ddVec3_In& translation = { 0.0f, 0.0f, 0.0f }) {
	const ddVec3 boxColor = { 0.0f, 0.8f, 0.8f };
	const ddVec3 boxCenter = { 0.0f, 0.0f, 3.0f };

	const ddMat4x4 quaternion = {
		(1.0 - 2.0 * q.y * q.y) - (2.0 * q.z * q.z),      (2.0 * q.x * q.y) - (2 * q.z * q.w),          (2.0 * q.x * q.z) + (2.0 * q.y * q.w), 0,
		(2.0 * q.x * q.y) + (2.0 * q.z * q.w)      ,      (1.0 - 2.0 * q.x * q.x) - (2.0 * q.z * q.z),  (2.0 * q.y * q.z) - (2.0 * q.x * q.w), 0, 
		(2.0 * q.x * q.z) - (2.0 * q.y * q.w)     ,	  (2 * q.y * q.z) + (2 * q.x * q.w),            (1.0 - 2.0 * q.x * q.x) - (2.0 * q.y * q.y), 0,
		translation[0], translation[1], translation[2],1
	};
	
	
	dd::axisTriad(quaternion, 3.0f, 6.0f);


}
int main(int, char**)
{
	
	// Create application window
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, _T("Hardlight Panel"), NULL };
	RegisterClassEx(&wc);
	HWND hwnd = CreateWindow(_T("Hardlight Panel"), _T("Hardlight Panel"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (CreateDeviceD3D(hwnd) < 0)
	{
		CleanupDeviceD3D();
		UnregisterClass(_T("ImGui Example"), wc.hInstance);
		return 1;
	}



	// Show the window
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	// Setup ImGui binding
	ImGui_ImplDX11_Init(hwnd, g_pd3dDevice, g_pd3dDeviceContext);


	ImVec4 clear_color = ImColor(114, 144, 154);

	
	
	assert(hvr_platform_isdllcompatible());

	hvr_platform* context;
	hvr_platform_create(&context);
	hvr_platform_startup(context);

	//Weird issue where there's a std::bad_alloc if these are started at nearly the same time. I think boost IPC shared memory is at the root
	//of it. Two threads allocating in the same process...same shared objects..

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	assert(HLVR_Version_IsCompatibleDLL());
	HLVR_System* plugin = nullptr;
	HLVR_System_Create(&plugin, nullptr);
	assert(HLVR_Version_IsCompatibleDLL());

	
	DirectXRenderInterface myDebugRenderer(nullptr);
	dd::initialize(&myDebugRenderer);

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	
	PlatformWindow platformWindow(context, plugin);


    while (msg.message != WM_QUIT)
    {
	

		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		ImGui_ImplDX11_NewFrame();

		ImGui::ShowTestWindow();

		ShowDriverInformation();

		HLVR_RuntimeInfo info;
		if (HLVR_OK(HLVR_System_GetRuntimeInfo(plugin, &info))) {
			ImGui::Text("Plugin has connected to runtime version %d.%d", info.MajorVersion, info.MinorVersion);
		}
		else {
			ImGui::Text("Plugin is not connected to runtime.");
		}
		platformWindow.Render();

	
		ImGui::Begin("Tracking");
			HLVR_TrackingUpdate t;
			if (HLVR_OK(HLVR_System_PollTracking(plugin, &t))) {
				myDebugRenderer.SetDrawlist(ImGui::GetWindowDrawList());


				ImVec2 chestOrigin = ImGui::GetCursorScreenPos();
				chestOrigin.x += 30.0f;
				chestOrigin.y += 20.0f;
				myDebugRenderer.SetOrigin(chestOrigin);

				ImGui::BeginChild("chest", ImVec2(120, 120), true);
					ImGui::Text("Chest");
					DrawQuaternionWidget(t.chest, { 3.0f, 3.9f, 0.0f });
				ImGui::EndChild();
				ImGui::SameLine();
				
				ImGui::BeginChild("lua", ImVec2(120, 120), true);
					ImGui::Text("Left UpperArm"); 
					DrawQuaternionWidget(t.left_upper_arm, { 15.0f, 3.9f, 0.0f });
				ImGui::EndChild();
				ImGui::SameLine();

				ImGui::BeginChild("rua", ImVec2(120, 120), true);
					ImGui::Text("Right UpperArm");
					DrawQuaternionWidget(t.right_upper_arm, { 28.0f, 3.9f, 0.0f });
				ImGui::EndChild();

			}
		ImGui::End();

		static int humDur = 0;
		static float humStrength = 1;
		ImGui::SliderInt("Waveform repetitions", &humDur, 0, 50);
		ImGui::SliderFloat("Waveform strength", &humStrength, 0, 1.0);
		std::vector<HLVR_Waveform> waveforms = { HLVR_Waveform_Bump, HLVR_Waveform_Buzz, HLVR_Waveform_Click, HLVR_Waveform_Fuzz, HLVR_Waveform_Hum, HLVR_Waveform_Pulse, HLVR_Waveform_Tick, HLVR_Waveform_Double_Click, HLVR_Waveform_Triple_Click };
		for (auto waveform : waveforms) {
			if (ImGui::Button(std::to_string(waveform).c_str())) {
				auto event = DiscreteHaptic_withregions(humDur, waveform, { hlvr_region_chest_left }, humStrength);
				HLVR_System_StreamEvent(plugin, event.get());
			}
		}
		
		if (ImGui::Button("Click")) {
			auto event = DiscreteHaptic_withregions(humDur, HLVR_Waveform_Click, { hlvr_region_chest_left });
			HLVR_System_StreamEvent(plugin, event.get());
		}
		if (ImGui::Button("Test pads sequentially")) {
			testPads(plugin);
		}

		static float click_interval = 0;
		static bool clicking_enabled = false;
		static std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
		if (ImGui::VSliderFloat("Click All Hz", ImVec2(25, 100), &click_interval, 0, 20.0, "%.1f")) {
			clicking_enabled = click_interval > 0;
		}

		if (clicking_enabled) {
			float cycle_time = 1.f / click_interval;
			auto delay = std::chrono::milliseconds(static_cast<int>(1000 * cycle_time));
			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastTime) > delay) {
				auto haptic = DiscreteHaptic_withregions(1, HLVR_Waveform_Click, { hlvr_region_body });
				HLVR_System_StreamEvent(plugin, haptic.get());
				lastTime = std::chrono::high_resolution_clock::now();
			}
		}
		
		if (ImGui::Button("Enable audio")) {
			HLVR_System_StreamEvent(plugin, EnableAudio({ hlvr_region_chest_left, hlvr_region_chest_right }).get());
		}

		if (ImGui::Button("Disable audio")) {
			HLVR_System_StreamEvent(plugin, DisableAudio({ hlvr_region_chest_left, hlvr_region_chest_right }).get());
		}

		if (ImGui::Button("Buffered")) {
			HLVR_System_StreamEvent(plugin, BufferedHaptic_withregions({ 0.0f, 0.1f, 0.2f, 0.3f, 0.5f, 0.6f, 0.8f, 1.0f, 1.0f, 0.5f, 0.2f, 0.0f }, 50, { hlvr_region_body }).get());
		}

		if (ImGui::Button("click")) {
			HLVR_System_StreamEvent(plugin, DiscreteHaptic_withregions(0, HLVR_Waveform_Hum, { hlvr_region_chest_right }).get());
		}


		
		dd::flush(0);
		
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
		ImGui::Render();

		g_pSwapChain->Present(1, 0); // Present with vsync

		
    }

	dd::shutdown();

	HLVR_System_Destroy(&plugin);

	hvr_platform_shutdown(context);
	hvr_platform_destroy(&context);

	ImGui_ImplDX11_Shutdown();

	CleanupDeviceD3D();
	UnregisterClass(_T("Hardlight Panel"), wc.hInstance);
    // Cleanup
  

    return 0;
}

