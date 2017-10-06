#pragma once

#include "NSDriverApi.h"
#include "NSLoader.h"
#include "NSLoader_Internal.h"
static void ShowHelpMarker(const char* desc);


class PlatformWindow {
public:
	PlatformWindow(hvr_platform* platform, NSVR_System* plugin);
	void Render();
	~PlatformWindow();
	

private:
	hvr_platform* m_platform;
	NSVR_System* m_plugin;

	NSVR_BodyView* m_retainedBodyview;
	void renderPlatformUI();
	void renderAvailableHardwarePlugins();

	void renderPlatformSide();
	void renderPluginSide();

	void renderEmulation();

};