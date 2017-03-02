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
	int PluginWrapper::CreateBasicHapticEvent(float time, float strength, float duration, uint32_t area, std::string effect);
	NSVR_InteropTrackingUpdate PollTracking();
	bool IsValidQuaternion(const NSVR_Quaternion& q);
	void Stop(unsigned int handle);
	int PollStatus();
	void SetTrackingEnabled(bool);
private:
	std::vector<char> readFromFile(std::string filename);
	NSVR_Context_t* m_plugin;
};

