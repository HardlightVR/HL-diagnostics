#pragma once

#include <stdint.h>

typedef int32_t NSVR_Result;


#ifndef NSVR_SUCCESS
#define NSVR_SUCCESS(result) (result >= 0)
#endif


#ifndef NSVR_FAILURE
#define NSVR_FAILURE(result) (!NSVR_SUCCESS(result))
#endif



typedef struct NSVR_ErrorInfo_ {
	NSVR_Result LastResult;
	char ErrorString[512];
} NSVR_ErrorInfo;


typedef enum NSVR_SuccessTypes_ {
	NSVR_Success_Unqualified = 0,
	
	/* Polling data */
	NSVR_Success_NoDataAvailable = 1000,

} NSVR_SuccessTypes;

typedef enum NSVR_ErrorTypes_ {


	/* General errors*/
	NSVR_Error_NullArgument = -1000,
	NSVR_Error_InvalidArgument = -1001,

	/* Creation errors */
	NSVR_Error_InvalidEventType = -2000,
} NSVR_ErrorTypes;

