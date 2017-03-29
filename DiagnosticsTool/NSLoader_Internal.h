#pragma once
#include <stdint.h>
#include "NSLoader_fwds.h"

#ifdef INTERNAL_TOOL
	#ifdef NSLOADER_EXPORTS
	#define NSLOADER_INTERNAL_API __declspec(dllexport) 
	#else
	#define NSLOADER_INTERNAL_API __declspec(dllimport) 
	#endif
#else 
	#define NSLOADER_INTERNAL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

	struct NSVR_LogEntry {
		char Message[512];
		uint32_t Length; 
	};

	NSLOADER_INTERNAL_API NSVR_Result __stdcall NSVR_System_PollLogs(NSVR_System* system, NSVR_LogEntry* entry);


	

#ifdef __cplusplus
}
#endif

