#include "buried_core.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks//stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "common/common_service.h"
#include "context/context.h"
#include "report/buried_report.h"
#include "third_party/nlohmann/json.hpp"

void Buried::m_InitWorkPath(const std::string& work_path) {
	std::filesystem::path t_work_path{ work_path };
	if (!std::filesystem::exists(t_work_path)) {
		std::filesystem::create_directories(t_work_path);
	}

	m_work_path = t_work_path / "buried";

	if (!std::filesystem::exists(m_work_path)) {
		std::filesystem::create_directories(m_work_path);
	}
}

void Buried::m_InitLogger() {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	std::filesystem::path log_dir = m_work_path / "buried.log";
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_dir.string(), true);

	m_logger = std::shared_ptr<spdlog::logger>{ new spdlog::logger("buried_sink",{console_sink,file_sink}) };

	m_logger->set_pattern("[%c] [%s:%#] [%l] %v");
	// time target.cpp:line debug/info msg
	m_logger->set_level(spdlog::level::trace);
}


std::shared_ptr<spdlog::logger> Buried::Logger() {
	return m_logger;
}

Buried::Buried(const std::string& work_path) {
	m_InitWorkPath(work_path);
	m_InitLogger();

	SPDLOG_LOGGER_INFO(Logger(), "Buried init success");
}

Buried::~Buried() {}
BuriedResult Buried::Start(const Config& config)
{
	buried::CommonService common_service;
	common_service.host = config.host;
	common_service.port = config.port;
	common_service.topic = config.topic;
	common_service.user_id = config.user_id;
	common_service.app_version = config.app_version;
	common_service.app_name = config.app_name;
	common_service.custom_data = nlohmann::json::parse(config.custom_data);

	m_buried_report = std::make_unique<buried::BuriedReport>(m_logger, std::move(common_service), m_work_path.string());
	m_buried_report->Start();
	return BuriedResult::kBuriedOk;
}
BuriedResult Buried::Report(std::string title, std::string data, uint32_t priority)
{
	buried::BuriedData buried_data;
	buried_data.title = std::move(title);
	buried_data.data = std::move(data);
	buried_data.priority = priority;
	m_buried_report->InsertData(buried_data);
	return BuriedResult::kBuriedOk;
}





