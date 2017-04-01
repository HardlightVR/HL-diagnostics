#pragma once

#include "NSLoader_Errors.h"

#include <stdint.h>

#ifdef NSLOADER_EXPORTS
#define NSLOADER_API __declspec(dllexport) 
#else
#define NSLOADER_API __declspec(dllimport) 
#endif

#define NSLOADER_API_VERSION_MAJOR 0
#define NSLOADER_API_VERSION_MINOR 4
#define NSLOADER_API_VERSION ((NSLOADER_API_VERSION_MAJOR << 16) | NSLOADER_API_VERSION_MINOR)




#ifdef __cplusplus
extern "C" {
#endif

	typedef struct NSVR_System_ NSVR_System;

	
	typedef struct NSVR_Timeline_ NSVR_Timeline;


	/*
	
	NSVR_Timeline* timeline;
	NSVR_Timeline_Create(timeline); //should system be injected here


	NSVR_Event* basicHaptic;
	NSVR_Event_Create(basicHaptic, NSVR_EventType_BasicHaptic);
	NSVR_Event_SetFloat(basicHaptic, "duration", 3.0f);
	NSVR_Event_SetInt(basicHaptic, "area", NSVR_AreaFlag_ChestLeft);


	NSVR_Timeline_Add(timeline, basicHaptic, 0.0); //should time be specified here

	NSVR_Timeline_Bind(system, timeline, handle); //or here

	//Future

	NSVR_Timeline_Attributes attribs;
	NSVR_Timeline_GetAttributes(timeline, &attribs);
	attribs.PlaybackSpeed = 0.5;
	attribs.Looping = true;
	attribs.LoopDelay = 0.15;
	NSVR_Timeline_SetAttributes(timeline, &attribs);
	
	
	*/
	//Will there be a time when the source doesn't contain time-indexed events?
	//Should NSVR_EventSequence_Add() take a time parameter to be explicit? 

	

	typedef enum NSVR_Effect_ {
		NSVR_Effect_Bump = 1,
		NSVR_Effect_Buzz = 2,
		NSVR_Effect_Click = 3,
		NSVR_Effect_Fuzz = 5,
		NSVR_Effect_Hum = 6,
		NSVR_Effect_Pulse = 8,
		NSVR_Effect_Tick = 11,
		NSVR_Effect_Double_Click = 4,
		NSVR_Effect_Triple_Click = 16,
		NSVR_Effect_Max = 4294967295
	} NSVR_Effect;



	struct NSVR_Quaternion {
		float w;
		float x;
		float y;
		float z;
	};
	struct NSVR_TrackingUpdate {
		NSVR_Quaternion chest;
		NSVR_Quaternion left_upper_arm;
		NSVR_Quaternion left_forearm;
		NSVR_Quaternion right_upper_arm;
		NSVR_Quaternion right_forearm;
	};

	struct NSVR_System_Status_ {
		int ConnectedToService;
		int ConnectedToSuit;
	};

	typedef struct NSVR_PlaybackHandle_ NSVR_PlaybackHandle;

	typedef struct NSVR_System_Status_ NSVR_System_Status;

	struct NSVR_Event_;
	typedef struct NSVR_Event_ NSVR_Event;
	typedef enum NSVR_PlaybackCommand_
	{
		NSVR_PlaybackCommand_Play = 0,
		NSVR_PlaybackCommand_Pause,
		NSVR_PlaybackCommand_Reset
	} NSVR_PlaybackCommand;

	typedef enum NSVR_EngineCommand_
	{
		NSVR_EngineCommand_ResumeAll = 1, 
		NSVR_EngineCommand_PauseAll, 
		NSVR_EngineCommand_DestroyAll, 
		NSVR_EngineCommand_EnableTracking, 
		NSVR_EngineCommand_DisableTracking
	} NSVR_EngineCommand;


	typedef enum NSVR_EventType_ {
		NSVR_EventType_BasicHapticEvent = 1,
		NSVR_EventType_Max = 65535
	} NSVR_EventType;




	//Creates a new instance of the plugin
	NSLOADER_API NSVR_Result __stdcall NSVR_System_Create(NSVR_System** systemPtr);

	//Destroys the plugin, releasing memory allocated to it
	NSLOADER_API void __stdcall NSVR_System_Release(NSVR_System** ptr);

	//Returns the version of this plugin, in the format ((Major << 16) | Minor)
	NSLOADER_API unsigned int __stdcall NSVR_GetVersion(void);

	//Returns true if the plugin is compatible with this header, false otherwise
	NSLOADER_API  int __stdcall NSVR_IsCompatibleDLL(void);


	//Commands the plugin, e.g. ENABLE_TRACKING
	NSLOADER_API NSVR_Result __stdcall NSVR_System_DoEngineCommand(NSVR_System* ptr, NSVR_EngineCommand command);

	//Returns true if a suit is plugged in and the service is running, else false
	NSLOADER_API NSVR_Result  __stdcall NSVR_System_PollStatus(NSVR_System* ptr, NSVR_System_Status* status);
	
	//Returns a structure containing quaternion tracking data
	NSLOADER_API NSVR_Result __stdcall NSVR_System_PollTracking(NSVR_System* ptr, NSVR_TrackingUpdate* q);
	

	NSLOADER_API NSVR_Result __stdcall NSVR_Event_Create(NSVR_Event** eventPtr, NSVR_EventType type);
	NSLOADER_API void __stdcall NSVR_Event_Release(NSVR_Event** event);
	NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetFloat(NSVR_Event* event, const char* key, float value);
	NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetInteger(NSVR_Event* event, const char* key, int value);



	NSLOADER_API NSVR_Result __stdcall NSVR_Timeline_Create(NSVR_Timeline** eventListPtr, NSVR_System* systemPtr);
	NSLOADER_API void __stdcall NSVR_Timeline_Release(NSVR_Timeline** listPtr);
	NSLOADER_API NSVR_Result __stdcall NSVR_Timeline_AddEvent(NSVR_Timeline* list, NSVR_Event* event);
	NSLOADER_API NSVR_Result __stdcall NSVR_Timeline_Transmit(NSVR_Timeline* timeline, NSVR_PlaybackHandle* handlePr);
	NSLOADER_API NSVR_Result __stdcall NSVR_PlaybackHandle_Create(NSVR_PlaybackHandle** handlePtr);
	NSLOADER_API NSVR_Result __stdcall NSVR_PlaybackHandle_Bind(NSVR_PlaybackHandle* handlePtr, NSVR_Timeline* timelinePtr);
	NSLOADER_API NSVR_Result __stdcall NSVR_PlaybackHandle_Command(NSVR_PlaybackHandle* handlePtr, NSVR_PlaybackCommand command);
	NSLOADER_API void __stdcall NSVR_PlaybackHandle_Release(NSVR_PlaybackHandle** handlePtr);




#ifdef __cplusplus
}
#endif

