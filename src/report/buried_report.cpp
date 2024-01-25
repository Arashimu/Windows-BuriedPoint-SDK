#include "report/buried_report.h"

#include <chrono>
#include <filesystem>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "context/context.h"
#include "crypt/crypt.h"
#include "database/database.h"
#include "report/http_report.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


namespace buried {
	static const char kDbName[] = "buried.db";

	class BuriedReportImpl {
	private:
		std::shared_ptr<spdlog::logger> m_logger; 
		std::string m_work_path;
		CommonService m_common_service;

		std::unique_ptr<BuriedDb> m_db;
		std::unique_ptr<Crypt> m_crypt;
		std::unique_ptr<boost::asio::deadline_timer> m_timer;

		std::vector<BuriedDb::Data>	m_data_caches;

		
	private:
		void m_Init();
		void m_ReportCache();
		void m_NextCycle();

		BuriedDb::Data m_MakeData(const BuriedData& data);

		std::string m_GenReportData(const std::vector<BuriedDb::Data>& datas);
		bool m_ReportData(const std::string& data);

	public:
		BuriedReportImpl(std::shared_ptr<spdlog::logger> logger,
			CommonService common_service,
			std::string work_path)
			:m_logger(std::move(logger)), m_work_path(std::move(work_path)), m_common_service(std::move(common_service)) {
			if (m_logger == nullptr) {
				m_logger = spdlog::stdout_color_mt("buried");
			}

			std::string key = AESCrypt::GetKey("buried_salt", "buried_password");
			m_crypt = std::make_unique<AESCrypt>(key);
			SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl init success");
	/*		Context::GetGlobalContext().GetReportStrand().post([this]() {
				m_Init();
				});*/
			m_Init();
		}
		~BuriedReportImpl() = default;

		void Start();
		void InsertData(const BuriedData& data);
	};



	void BuriedReportImpl::m_Init()
	{
		std::filesystem::path db_path = m_work_path;
		SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl init db path: {}", db_path.string());

		db_path /= kDbName;
		m_db = std::make_unique<BuriedDb>(db_path.string());
	}

	void BuriedReportImpl::m_ReportCache()
	{
		SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl report cache");
		if (m_data_caches.empty()) {
			m_data_caches = m_db->QueryData(10);
		}
		if (!m_data_caches.empty()) {
			std::string report_data = m_GenReportData(m_data_caches);
			if (m_ReportData(report_data)) {
				m_db->DeleteDatas(m_data_caches);
				m_data_caches.clear();
			}
		}
		m_NextCycle();
	}

	void BuriedReportImpl::m_NextCycle()
	{
		SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl next cycle");
		m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(5));
		m_timer->async_wait([this](const boost::system::error_code& ec) {
			if (ec) {
				m_logger->error("BuriedReportImpl::NextCycle_ error: {}", ec.message());
				return;
			}
			Context::GetGlobalContext().GetReportStrand().post(
				[this]() { m_ReportCache(); });
			});
	}

	BuriedDb::Data BuriedReportImpl::m_MakeData(const BuriedData& data)
	{
		BuriedDb::Data db_data;
		db_data.id = -1;
		db_data.priority = data.priority;
		db_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
			).count();

		nlohmann::json json_data;
		json_data["title"] = data.title;
		json_data["data"] = data.data;
		json_data["user_id"] = m_common_service.user_id;
		json_data["app_version"] = m_common_service.app_version;
		json_data["app_name"] = m_common_service.app_name;
		json_data["custom_data"] = m_common_service.custom_data;
		json_data["system_version"] = m_common_service.system_version;
		json_data["device_name"] = m_common_service.device_name;
		json_data["device_id"] = m_common_service.device_id;
		json_data["buried_version"] = m_common_service.buried_version;
		json_data["lifecycle_id"] = m_common_service.lifecycle_id;
		json_data["priority"] = data.priority;
		json_data["timestamp"] = CommonService::GetNowDate();
		json_data["process_time"] = CommonService::GetProcessTime();
		json_data["report_id"] = CommonService::GetRandomId();

		std::string report_data = m_crypt->Encrypt(json_data.dump());
		db_data.content = std::vector<char>(report_data.begin(), report_data.end());

		SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl insert data size: {}",
			db_data.content.size());
		return db_data;
	}

	std::string BuriedReportImpl::m_GenReportData(const std::vector<BuriedDb::Data>& datas)
	{
		nlohmann::json json_datas;
		for (const auto& data : datas) {
			std::string  content = m_crypt->Decrypt(data.content.data(), data.content.size());
			SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl report data content size: {}", data.content.size());
			json_datas.push_back(content);
		}

		std::string ret = json_datas.dump();
		return ret;
	}

	bool BuriedReportImpl::m_ReportData(const std::string& data)
	{
		HttpReporter reporter{ m_logger };
		return reporter.Host(m_common_service.host)
			.Tpoic(m_common_service.topic)
			.Port(m_common_service.port)
			.Body(data)
			.Report();
	}

	void BuriedReportImpl::Start()
	{
		SPDLOG_LOGGER_INFO(m_logger, "BuriedReportImpl start");

		m_timer = std::make_unique<boost::asio::deadline_timer>(
			Context::GetGlobalContext().GetMainContext(),
			boost::posix_time::seconds(5)
			);

		m_timer->async_wait(Context::GetGlobalContext().GetReportStrand().wrap(
			[this](const boost::system::error_code& ec) {
				if (ec) {
					m_logger->error("BuriedReportImpl::Start error: {}", ec.message());
				}
				m_ReportCache();
			}
		));
	}

	void BuriedReportImpl::InsertData(const BuriedData& data)
	{
		Context::GetGlobalContext().GetReportStrand().post(
			[this, data]() {
				m_db->InsertData(m_MakeData(data));
			}
		);
	}

	BuriedReport::BuriedReport(std::shared_ptr<spdlog::logger>logger, CommonService common_service, std::string work_path)
		:m_pimpl(std::make_unique<BuriedReportImpl>(
			std::move(logger),std::move(common_service),std::move(work_path)))
	{}

	BuriedReport::~BuriedReport(){}

	void BuriedReport::Start()
	{
		m_pimpl->Start();
	}


	void BuriedReport::InsertData(const BuriedData& data)
	{
		m_pimpl->InsertData(data);
	}

}