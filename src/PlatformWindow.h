#pragma once

#include "NSDriverApi.h"

static void ShowHelpMarker(const char* desc);


class PlatformWindow {
public:
	PlatformWindow(hvr_platform* platform);
	void Render();

	

private:
	hvr_platform* m_platform;
	void renderPlatformUI();
	void renderPluginUI();

};