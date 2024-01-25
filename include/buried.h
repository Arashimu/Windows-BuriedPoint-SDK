#pragma once

#include <stdint.h>

#define BURIED_EXPORT __declspec(dllexport)


extern "C" {
	typedef struct Buried Buried;
	struct  BuriedConfig
	{
		const char* host;
		const char* port;
		const char* topic;
		const char* user_id;
		const char* app_version;
		const char* app_name;
		const char* custom_data;
	};

	BURIED_EXPORT Buried* BuriedCreate(const char* work_dir);
	BURIED_EXPORT void BuriedDestroy(Buried* buried);
	BURIED_EXPORT int32_t BuriedStart(Buried* buried, BuriedConfig* config);
	BURIED_EXPORT int32_t BuriedReport(Buried* buried, const char* title, const char* data, uint32_t priotiy);
}
