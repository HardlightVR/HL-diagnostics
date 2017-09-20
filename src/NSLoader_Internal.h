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
#define NSVR_RETURN_INTERNAL(ReturnType) NSLOADER_INTERNAL_API ReturnType __cdecl

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


	/* Immediate API */
	typedef struct NSVR_BodyView NSVR_BodyView;


	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_Immediate_Set(NSVR_System* systemPtr, uint32_t* regions, double* amplitudes, uint32_t length);

	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_Create(NSVR_BodyView** body);
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_Release(NSVR_BodyView** body);
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_Poll(NSVR_BodyView* body, NSVR_System* system);
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetNodeCount(NSVR_BodyView* body, uint32_t* outNodeCount);
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetNodeType(NSVR_BodyView * body, uint32_t nodeIndex, uint32_t* outType);
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetNodeRegion(NSVR_BodyView * body, uint32_t nodeIndex, uint32_t* outRegion);

	//only valid if nodeType == intensity
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetIntensity(NSVR_BodyView * body, uint32_t nodeIndex, float* outIntensity);

	typedef struct NSVR_Color {
		float r;
		float g;
		float b;
		float a;
	} NSVR_Color;
	//only valid if nodeType == color
	NSVR_RETURN_INTERNAL(NSVR_Result) NSVR_BodyView_GetColor(NSVR_BodyView * body, uint32_t nodeIndex, NSVR_Color* outColor);
#ifdef __cplusplus
}
#endif

