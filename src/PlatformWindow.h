#pragma once

#include "NSDriverApi.h"
#include "HLVR.h"
#include "HLVR_Experimental.h"
static void ShowHelpMarker(const char* desc);


class PlatformWindow {
public:
	PlatformWindow(hvr_platform* platform, HLVR_System* plugin);
	void Render();
	~PlatformWindow();
	

private:
	hvr_platform* m_platform;
	HLVR_System* m_plugin;

	HLVR_BodyView* m_retainedBodyview;
	void renderPlatformUI();
	void renderAvailableHardwarePlugins();

	void renderPlatformSide();
	void renderPluginSide();

	void renderEmulation();

};