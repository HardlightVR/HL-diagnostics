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


#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "NSDriverApi.h"
#include "PlatformWindow.h"
#include "HLVR.h"


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


void ShowDriverInformation() {
	ImGui::Begin("Platform Info");
	{
		std::stringstream s;
		auto version = hvr_platform_getversion();
		s << "HardlightPlatform.dll version " << (version >> 16) << "." << ((version << 16) >> 16);
		ImGui::Text(s.str().c_str());

		auto pluginVersion = HLVR_Version_Get();
		std::stringstream s2;
		s2<< "Hardlight.dll version " << (pluginVersion >> 16) << "." << ((pluginVersion << 16) >> 16);
		ImGui::Text(s2.str().c_str());
	}
	ImGui::End();
}


void testPads(HLVR_Agent* system) {


	HLVR_NodeIterator nodeIter;
	HLVR_NodeIterator_Init(&nodeIter);

	HLVR_Timeline* timeline;
	HLVR_Timeline_Create(&timeline);

	float timeOffset = 0.0f;
	while (HLVR_NodeIterator_Next(&nodeIter, 0, system)) {
		HLVR_Event* event;
		HLVR_Event_Create(&event, HLVR_EventType_SimpleHaptic);
		HLVR_Event_SetUInt32s(event, HLVR_EventKey_SimpleHaptic_Nodes_UInt32s, (uint32_t*)(&nodeIter.NodeInfo.Id), 1);
		HLVR_Event_SetInt(event, HLVR_EventKey_SimpleHaptic_Effect_Int, HLVR_Waveform_Hum);
		HLVR_Event_SetFloat(event, HLVR_EventKey_SimpleHaptic_Duration_Float, 1.0);
		HLVR_Event_SetFloat(event, HLVR_EventKey_Time_Float, timeOffset);
		HLVR_Timeline_AddEvent(timeline, event);
		HLVR_Event_Destroy(&event);

		timeOffset += 1.0f;
	}
	HLVR_Effect* handle;
	HLVR_Effect_Create(&handle);
	HLVR_Timeline_Transmit(timeline, system, handle);

	HLVR_Timeline_Destroy(&timeline);

	HLVR_Effect_Play(handle);
	HLVR_Effect_Destroy(&handle);
}

int main(int, char**)
{
	
	// Create application window
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, _T("ImGui Example"), NULL };
	RegisterClassEx(&wc);
	HWND hwnd = CreateWindow(_T("ImGui Example"), _T("Hardlight Platform Diagnostics"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

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


	HLVR_Agent* plugin = nullptr;
	HLVR_Agent_Create(&plugin, nullptr);
	assert(HLVR_Version_IsCompatibleDLL());

	
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

		platformWindow.Render();


		if (ImGui::Button("TEST")) {
			testPads(plugin);
		}

		
		
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
		ImGui::Render();

		g_pSwapChain->Present(1, 0); // Present with vsync

		
    }

	HLVR_Agent_Destroy(&plugin);

	hvr_platform_shutdown(context);
	hvr_platform_destroy(&context);

	ImGui_ImplDX11_Shutdown();

	CleanupDeviceD3D();
	UnregisterClass(_T("ImGui Example"), wc.hInstance);
    // Cleanup
  

    return 0;
}

