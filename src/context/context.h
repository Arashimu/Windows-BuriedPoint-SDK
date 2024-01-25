#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "boost/asio/io_context.hpp"
#include "boost/asio/io_context_strand.hpp"

namespace buried {
	class Context {
	public:
		static Context& GetGlobalContext() {
			static Context global_context;
			return global_context;
		}

		~Context();

		using Strand = boost::asio::io_context::strand;
		using IOContext = boost::asio::io_context;

		Strand& GetMainStrand() { return m_main_strand; }
		Strand& GetReportStrand() { return m_report_strand; }
		IOContext& GetMainContext() { return m_main_context; }

		void Start();
	private:
		boost::asio::io_context m_main_context;
		boost::asio::io_context m_report_context;

		boost::asio::io_context::strand m_main_strand;
		boost::asio::io_context::strand m_report_strand;

		std::unique_ptr<std::thread> m_main_thread;
		std::unique_ptr<std::thread> m_report_thread;

		std::atomic<bool> m_is_start{ false };
		std::atomic<bool> m_is_stop{ false };
	private:
		Context():m_main_strand(m_main_context),m_report_strand(m_report_context){}
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
	};
}