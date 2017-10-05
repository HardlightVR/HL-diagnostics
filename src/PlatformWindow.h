#pragma once

#include "NSDriverApi.h"
#include "NSLoader.h"
static void ShowHelpMarker(const char* desc);


class PlatformWindow {
public:
	PlatformWindow(hvr_platform* platform, NSVR_System* plugin);
	void Render();

	

private:
	hvr_platform* m_platform;
	NSVR_System* m_plugin;
	void renderPlatformUI();
	void renderAvailableHardwarePlugins();

	void renderPlatformSide();
	void renderPluginSide();

};