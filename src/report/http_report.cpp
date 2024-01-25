#include <exception>


#include "report/http_report.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/version.hpp"
#include "spdlog/spdlog.h"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace buried {
	static net::io_context ioc;

	HttpReporter::HttpReporter(std::shared_ptr<spdlog::logger> logger): m_logger(logger){}

	bool HttpReporter::Report() {
		try {
			int version = 11;

			tcp::resolver resolver{ ioc };
			beast::tcp_stream stream{ioc};

			tcp::resolver::query query{ m_host,m_port };

			auto const results = resolver.resolve(query);

			stream.connect(results);

			http::request<http::string_body> req{ http::verb::post,m_topic,version };
			req.set(http::field::host, m_host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
			req.set(http::field::content_type, "application/json");
			req.body() = m_body;
			req.prepare_payload();

			http::write(stream, req);

			beast::flat_buffer buffer;

			http::response<http::dynamic_body> res;

			http::read(stream, buffer, res);

			beast::error_code ec;

			stream.socket().shutdown(tcp::socket::shutdown_both, ec);

			if (ec && ec != beast::errc::not_connected) throw beast::system_error{ ec };

			auto res_status = res.result();
			if (res_status != http::status::ok) {
				SPDLOG_LOGGER_ERROR(m_logger, "report error " + std::to_string(res.result_int()));
				return false;
			}

			std::string res_body = beast::buffers_to_string(res.body().data());

			SPDLOG_LOGGER_TRACE(m_logger, "report success " + res_body);
		}
		catch (std::exception const& e) {
			SPDLOG_LOGGER_TRACE(m_logger, "report error " + std::string(e.what()));
			return false;
		}
		return true;
	}
}