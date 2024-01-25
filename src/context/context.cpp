#include "context/context.h"

namespace buried {
	void Context::Start() {
		if (m_is_start.load()) {
			return;
		}

		m_is_start.store(true);

		m_main_thread = std::make_unique<std::thread>([this]() {
			for (;;) {
				if (m_is_stop) {
					break;
				}
				m_main_context.run();
			}
			});
		m_report_thread = std::make_unique<std::thread>([this]() {
			for (;;) {
				if (m_is_stop) {
					break;
				}
				m_report_context.run();
			}
			});
	}

	Context::~Context() {
		m_is_stop = true;
		if (m_main_thread) {
			m_main_thread->join();
		}
		if (m_report_thread) {
			m_report_thread->join();
		}
	}
}