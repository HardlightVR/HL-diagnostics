#pragma once

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "NSLoader.h"

#include <string>
#include <vector>
class PluginWrapper
{
public:
	PluginWrapper();
	~PluginWrapper();
	int Play(unsigned int handle);
	int Create(std::string hapticFileName);
	void Stop(unsigned int handle);
	int PollStatus();
	void SetTrackingEnabled(bool);
private:
	std::vector<char> readFromFile(std::string filename);
	NSVRPlugin m_plugin;
};

