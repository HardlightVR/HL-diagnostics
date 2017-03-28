#include "stdafx.h"
#include "PluginWrapper.h"
#include <fstream>
#include "flatbuffers/flatbuffers.h"
#include "Events_generated.h"
#include <iostream>
PluginWrapper::PluginWrapper()
{
	m_plugin = NSVR_Create();

	if (!NSVR_IsCompatibleDLL()) {
		std::cout << "Incompatible NSVR Plugin DLL. Email casey@nullspacevr.com" << '\n';
	}
}


PluginWrapper::~PluginWrapper()
{
	NSVR_Delete(m_plugin);
}

int PluginWrapper::Play(unsigned int handle)
{
	NSVR_DoHandleCommand(m_plugin, handle, NSVR_HandleCommand::PLAY);
	return handle;

}

int PluginWrapper::CreateBasicHapticEvent(float time, float strength, float duration, uint32_t area, std::string effect) {
	flatbuffers::FlatBufferBuilder builder;
	auto feffect = builder.CreateString(effect);
	auto effectOffset = NullSpace::Events::CreateBasicHapticEvent(builder, time, strength, duration, area, 0);
	
	auto suitEvent = NullSpace::Events::CreateSuitEvent(builder, NullSpace::Events::SuitEventType_BasicHapticEvent, effectOffset.Union());

	std::vector<flatbuffers::Offset<NullSpace::Events::SuitEvent>> stuff;
	stuff.push_back(suitEvent);

	auto listOffset = builder.CreateVector(stuff);
	auto finalList = NullSpace::Events::CreateSuitEventList(builder, listOffset);
	builder.Finish(finalList);

	unsigned int handle = NSVR_GenHandle(m_plugin);
	NSVR_TransmitEvents(m_plugin, handle, builder.GetBufferPointer(), builder.GetSize());
	return handle;

}
int PluginWrapper::Create(std::string hapticFileName)
{
	unsigned int handle = 0;
	try {
		auto data = readFromFile(hapticFileName);
		handle = NSVR_GenHandle(m_plugin);
		//NSVR_CreateHaptic(m_plugin, handle, (void*)data.data(), data.size());
	
	}
	catch (std::runtime_error e) {
		return 0;
	}

	return handle;
}

NSVR_InteropTrackingUpdate PluginWrapper::PollTracking()
{
	NSVR_InteropTrackingUpdate update;
	NSVR_PollTracking(m_plugin, update);
	return update;
}

bool PluginWrapper::IsValidQuaternion(const NSVR_Quaternion & q)
{
	return !(abs(q.w) < 0.00001 && abs(q.x) < 0.00001 && abs(q.y) < 0.00001 && abs(q.z) < 0.00001);
}

void PluginWrapper::Stop(unsigned int handle)
{
	NSVR_DoHandleCommand(m_plugin, handle, NSVR_HandleCommand::RESET);
}

int PluginWrapper::PollStatus()
{
	return NSVR_PollStatus(m_plugin);
}

void PluginWrapper::SetTrackingEnabled(bool wantTracking)
{
	if (wantTracking) {
		NSVR_DoEngineCommand(m_plugin, NSVR_EngineCommand::ENABLE_TRACKING);
	}
	else {
		NSVR_DoEngineCommand(m_plugin, NSVR_EngineCommand::DISABLE_TRACKING);
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

