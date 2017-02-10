#include "stdafx.h"
#include "PluginWrapper.h"
#include <fstream>
PluginWrapper::PluginWrapper()
{
	m_plugin = NSVR_Create();
}


PluginWrapper::~PluginWrapper()
{
	NSVR_Delete(m_plugin);
}

int PluginWrapper::Play(unsigned int handle)
{
	NSVR_HandleCommand(m_plugin, handle, 0);
	return handle;

}

int PluginWrapper::Create(std::string hapticFileName)
{
	unsigned int handle = 0;
	try {
		auto data = readFromFile(hapticFileName);
		handle = NSVR_GenHandle(m_plugin);
		NSVR_CreateHaptic(m_plugin, handle, (void*)data.data(), data.size());
	
	}
	catch (std::runtime_error e) {
		return 0;
	}

	return handle;
}

InteropTrackingUpdate PluginWrapper::PollTracking()
{
	InteropTrackingUpdate update;
	NSVR_PollTracking(m_plugin, update);
	return update;
}

bool PluginWrapper::IsValidQuaternion(const Quaternion & q)
{
	return !(abs(q.w) < 0.00001 && abs(q.x) < 0.00001 && abs(q.y) < 0.00001 && abs(q.z) < 0.00001);
}

void PluginWrapper::Stop(unsigned int handle)
{
	NSVR_HandleCommand(m_plugin, handle, 2);
}

int PluginWrapper::PollStatus()
{
	return NSVR_PollStatus(m_plugin);
}

void PluginWrapper::SetTrackingEnabled(bool wantTracking)
{
	if (wantTracking) {
		NSVR_EngineCommand(m_plugin, 4);
	}
	else {
		NSVR_EngineCommand(m_plugin, 5);
	}
}

std::vector<char> PluginWrapper::readFromFile(std::string filename)
{

	
	using namespace std;
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) {
		throw std::runtime_error("Couldn't find the file");
	}
	// copies all data into buffer
	std::vector<char> buffer((
		std::istreambuf_iterator<char>(input)),
		(std::istreambuf_iterator<char>()));
	return buffer;

	
}

