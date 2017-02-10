#pragma once


#ifdef NSLOADER_EXPORTS
#define NSLOADER_API __declspec(dllexport) 
#else
#define NSLOADER_API __declspec(dllimport) 
#endif

#define NSVR_VERSION "v0.3.0-rc1"

struct _NSVRPlugin;
typedef struct _NSVRPlugin* NSVRPlugin;


namespace NullSpaceDLL {
	struct Quaternion {
		float w;
		float x;
		float y;
		float z;
	};
	struct InteropTrackingUpdate {
		Quaternion chest;
		Quaternion left_upper_arm;
		Quaternion left_forearm;
		Quaternion right_upper_arm;
		Quaternion right_forearm;
	};

	
}

extern "C" {
	

	NSLOADER_API NSVRPlugin __stdcall NSVR_Create();
	NSLOADER_API void __stdcall NSVR_Delete(NSVRPlugin ptr);

	NSLOADER_API unsigned int __stdcall NSVR_GenHandle(NSVRPlugin ptr);
	NSLOADER_API int  __stdcall NSVR_PollStatus(NSVRPlugin ptr);

	NSLOADER_API void __stdcall NSVR_PollTracking(NSVRPlugin ptr, NullSpaceDLL::InteropTrackingUpdate& q);
	NSLOADER_API bool __stdcall NSVR_Load(NSVRPlugin ptr, LPSTR param, int fileType);
	NSLOADER_API bool __stdcall NSVR_CreateSequence(NSVRPlugin ptr, unsigned int handle, LPSTR name, unsigned int loc);

	NSLOADER_API bool __stdcall NSVR_CreatePattern(NSVRPlugin ptr, unsigned int handle, LPSTR name);

	NSLOADER_API bool __stdcall NSVR_CreateExperience(NSVRPlugin ptr, unsigned int handle, LPSTR name);

	NSLOADER_API void __stdcall NSVR_HandleCommand(NSVRPlugin ptr, unsigned int handle, short command);

	NSLOADER_API char* __stdcall NSVR_GetError(NSVRPlugin ptr);

	NSLOADER_API void __stdcall NSVR_FreeString(char* string);

	NSLOADER_API bool __stdcall NSVR_EngineCommand(NSVRPlugin ptr, short command);

	NSLOADER_API bool __stdcall NSVR_InitializeFromFilesystem(NSVRPlugin ptr, LPSTR path);

	NSLOADER_API void __stdcall NSVR_CreateHaptic(NSVRPlugin ptr, unsigned int handle, void* data, unsigned int size);
}