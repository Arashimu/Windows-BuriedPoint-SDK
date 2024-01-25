#include "include/buried.h"

#include <iostream>
#include "buried_core.h"

extern "C" {
	Buried* BuriedCreate(const char* work_path) {
		if (work_path == nullptr) {
			return nullptr;
		}
		return new Buried(work_path);
	}

	void BuriedDestroy(Buried* buried) {
		if (buried!=nullptr) {
			delete buried;
		}
	}


	int32_t BuriedStart(Buried* buried,BuriedConfig* config) {
		if (buried == nullptr || config == nullptr) {
			return static_cast<int32_t>(BuriedResult::kBuriedInvaildParam);
		}

		Buried::Config buried_config;
		if (config->host) {
			buried_config.host = config->host;
		}

		if (config->port) {
			buried_config.port = config->port;
		}
		if (config->topic) {
			buried_config.topic = config->topic;
		}
		if (config->user_id) {
			buried_config.user_id = config->user_id;
		}
		if (config->app_version) {
			buried_config.app_version = config->app_version;
		}
		if (config->app_name) {
			buried_config.app_name = config->app_name;
		}
		if (config->custom_data) {
			buried_config.custom_data = config->custom_data;
		}
		return static_cast<int32_t>(buried->Start(buried_config));
	}
	int32_t BuriedReport(Buried* buried, const char* title, const char* data, uint32_t priotiy) {
		if (buried == nullptr || title == nullptr || data == nullptr) {
			return static_cast<int32_t>(BuriedResult::kBuriedInvaildParam);
		}
		return static_cast<int32_t>(buried->Report(title, data, priotiy));
	}

}