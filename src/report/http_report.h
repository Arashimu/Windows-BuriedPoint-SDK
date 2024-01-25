#pragma once

#include <stdint.h>
#include <memory>
#include <string>

namespace spdlog {
	class logger;
}

namespace buried {
	class HttpReporter {
	private:
		std::string m_host;
		std::string m_topic;
		std::string m_port;
		std::string m_body;

		std::shared_ptr<spdlog::logger> m_logger;
	public:
		explicit HttpReporter(std::shared_ptr<spdlog::logger> logger);

		//建造模式
		HttpReporter& Host(const std::string& host) {
			m_host = host;
			return *this;
		}

		HttpReporter& Tpoic(const std::string& topic) {
			m_topic = topic;
			return *this;
		}

		HttpReporter& Port(const std::string& port) {
			m_port = port;
			return *this;
		}

		HttpReporter& Body(const std::string& body) {
			m_body = body;
			return *this;
		}

		bool Report();
	};
}