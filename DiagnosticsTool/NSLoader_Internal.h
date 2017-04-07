#pragma once
#include <stdint.h>

#include "NSLoader_fwds.h"

#define INTERNAL_TOOL

#ifdef INTERNAL_TOOL
	#ifdef NSLOADER_EXPORTS
	#define NSLOADER_INTERNAL_API __declspec(dllexport) 
	#else
	#define NSLOADER_INTERNAL_API __declspec(dllimport) 
	#endif
#else 
	#define NSLOADER_INTERNAL_API
#endif
#define NSVR_RETURN_INTERNAL(ReturnType) NSLOADER_INTERNAL_API ReturnType __stdcall

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct NSVR_LogEntry_ {
		char Message[512];
		uint32_t Length; 
	} NSVR_LogEntry;

	typedef struct NSVR_AudioOptions_ {
		uint8_t AudioMax;
		uint8_t AudioMin;
		uint8_t PeakTime;
		uint8_t Filter;
	} NSVR_AudioOptions;

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_PollLogs(NSVR_System* system, NSVR_LogEntry* entry);

	typedef struct NSVR_SystemStats_ {
		unsigned int NumLiveEffects;
		unsigned int NumOrphanedEffects;
	} NSVR_SystemStats;
	

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_GetStats(NSVR_System* system, NSVR_SystemStats* ptr);

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_Audio_Enable(NSVR_System* system, NSVR_AudioOptions* optionsPtr);

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_Audio_Disable(NSVR_System* system);

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_SubmitRawCommand(NSVR_System* system, uint8_t* buffer, int length);

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_System_DumpDeviceDiagnostics(NSVR_System* system);

#ifdef __cplusplus
}
#endif

