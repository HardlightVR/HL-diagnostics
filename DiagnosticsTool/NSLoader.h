#pragma once
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

	struct NSVR_System_;
	typedef struct NSVR_System_ NSVR_System;

	struct NSVR_EventList_;
	typedef struct NSVR_EventList_ NSVR_EventList;

	typedef enum NSVR_Effect_ {
		Bump = 1,
		Buzz = 2,
		Click = 3,
		Fuzz = 5,
		Hum = 6,
		Pulse = 8,
		Tick = 11,
		Double_Click = 4,
		Triple_Click = 16,
		NSVR_Effect_MAX = 4294967295
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

	typedef struct NSVR_System_Status_ NSVR_System_Status;

	struct NSVR_Event_;
	typedef struct NSVR_Event_ NSVR_Event;
	enum NSVR_HandleCommand
	{
		PLAY = 0, PAUSE, RESET, RELEASE
	};

	enum NSVR_EngineCommand
	{
		RESUME_ALL = 1, PAUSE_ALL, DESTROY_ALL, ENABLE_TRACKING, DISABLE_TRACKING
	};

	enum NSVR_ConnectionStatus
	{
		DISCONNECTED = 0,
		CONNECTED = 2
	};

	enum NSVR_EventType {
		BASIC_HAPTIC_EVENT = 1,
		NSVR_EventType_MAX = 65535
	};


	typedef struct NSVR_ErrorInfo_ {
		char ErrorString[512];
	} NSVR_ErrorInfo;

	typedef int32_t NSVR_Result;

	//Creates a new instance of the plugin
	NSLOADER_API NSVR_System* __stdcall NSVR_System_Create();


	//Destroys the plugin, releasing memory allocated to it
	NSLOADER_API void __stdcall NSVR_System_Release(NSVR_System* ptr);

	//Returns the version of this plugin, in the format ((Major << 16) | Minor)
	NSLOADER_API unsigned int __stdcall NSVR_GetVersion(void);

	//Returns true if the plugin is compatible with this header, false otherwise
	NSLOADER_API  int __stdcall NSVR_IsCompatibleDLL(void);

	//Generates a unique handle to refer to the data sent in NSVR_TransmitEvents
	NSLOADER_API unsigned int __stdcall NSVR_System_GenerateHandle(NSVR_System* ptr);

	//Manipulates the effect associated with the given handle by specifying a command
	NSLOADER_API void __stdcall NSVR_System_DoHandleCommand(NSVR_System* ptr, uint32_t handle, NSVR_HandleCommand command);

	//Commands the plugin, e.g. ENABLE_TRACKING
	NSLOADER_API NSVR_Result __stdcall NSVR_System_DoEngineCommand(NSVR_System* ptr, NSVR_EngineCommand command);

	//Returns true if a suit is plugged in and the service is running, else false
	NSLOADER_API NSVR_Result  __stdcall NSVR_System_PollStatus(NSVR_System* ptr, NSVR_System_Status* status);
	
	//Returns a structure containing quaternion tracking data
	NSLOADER_API void __stdcall NSVR_System_PollTracking(NSVR_System* ptr, NSVR_TrackingUpdate* q);
	
	//Retrieves a newly-allocated string containing the latest error information. 
	//Must be deallocated using NSVR_FreeError
	NSLOADER_API char* __stdcall NSVR_System_GetError(NSVR_System* ptr);

	//Deallocates the memory pointed to by string. Only pass pointers returned by NSVR_GetError.
	NSLOADER_API void __stdcall NSVR_FreeError(char* string);


	NSLOADER_API NSVR_Event* __stdcall NSVR_Event_Create(NSVR_EventType type);
	NSLOADER_API void __stdcall NSVR_Event_Release(NSVR_Event* event);
	NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetFloat(NSVR_Event* event, const char* key, float value);
	NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetInteger(NSVR_Event* event, const char* key, int value);



	NSLOADER_API NSVR_EventList* __stdcall NSVR_EventList_Create();
	NSLOADER_API void __stdcall NSVR_EventList_Release(NSVR_EventList* listPtr);
	NSLOADER_API NSVR_Result __stdcall NSVR_EventList_AddEvent(NSVR_EventList* list, NSVR_Event* event);
	NSLOADER_API NSVR_Result __stdcall NSVR_EventList_Bind(NSVR_System* ptr, NSVR_EventList* listPtr, uint32_t handle);

	



#ifdef __cplusplus
}
#endif

