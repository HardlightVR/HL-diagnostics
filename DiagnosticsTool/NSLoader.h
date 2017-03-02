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

	struct NSVR_Context;
	typedef struct NSVR_Context NSVR_Context_t;

	struct NSVR_Quaternion {
		float w;
		float x;
		float y;
		float z;
	};
	struct NSVR_InteropTrackingUpdate {
		NSVR_Quaternion chest;
		NSVR_Quaternion left_upper_arm;
		NSVR_Quaternion left_forearm;
		NSVR_Quaternion right_upper_arm;
		NSVR_Quaternion right_forearm;
	};


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

	//Creates a new instance of the plugin
	NSLOADER_API NSVR_Context_t* __stdcall NSVR_Create();

	//Destroys the plugin, releasing memory allocated to it
	NSLOADER_API void __stdcall NSVR_Delete(NSVR_Context_t* ptr);

	//Returns the version of this plugin, in the format ((Major << 16) | Minor)
	NSLOADER_API unsigned int __stdcall NSVR_GetVersion(void);

	//Returns true if the plugin is compatible with this header, false otherwise
	NSLOADER_API  int __stdcall NSVR_IsCompatibleDLL(void);

	//Generates a unique handle to refer to the data sent in NSVR_TransmitEvents
	NSLOADER_API unsigned int __stdcall NSVR_GenHandle(NSVR_Context_t* ptr);

	//Transmits a list of flatbuffer encoded events to the plugin, binding them to the given handle
	NSLOADER_API int __stdcall NSVR_TransmitEvents(NSVR_Context_t* ptr, uint32_t handle, void* data, uint32_t size);

	//Manipulates the effect associated with the given handle by specifying a command
	NSLOADER_API void __stdcall NSVR_DoHandleCommand(NSVR_Context_t* ptr, uint32_t handle, NSVR_HandleCommand command);

	//Commands the plugin, e.g. ENABLE_TRACKING
	NSLOADER_API int __stdcall NSVR_DoEngineCommand(NSVR_Context_t* ptr, NSVR_EngineCommand command);

	//Returns true if a suit is plugged in and the service is running, else false
	NSLOADER_API int  __stdcall NSVR_PollStatus(NSVR_Context_t* ptr);
	
	//Returns a structure containing quaternion tracking data
	NSLOADER_API void __stdcall NSVR_PollTracking(NSVR_Context_t* ptr, NSVR_InteropTrackingUpdate& q);
	
	//Retrieves a newly-allocated string containing the latest error information. 
	//Must be deallocated using NSVR_FreeError
	NSLOADER_API char* __stdcall NSVR_GetError(NSVR_Context_t* ptr);

	//Deallocates the memory pointed to by string. Only pass pointers returned by NSVR_GetError.
	NSLOADER_API void __stdcall NSVR_FreeError(char* string);

	
#ifdef __cplusplus
}
#endif

