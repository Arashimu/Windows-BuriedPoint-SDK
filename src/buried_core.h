#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "buried_common.h"
#include "include/buried.h"


namespace spdlog{
    class logger;
}

namespace buried{
    class BuriedReport;
}


struct Buried {
public:
    struct Config {
        std::string host;
        std::string port;
        std::string topic;
        std::string user_id;
        std::string app_version;
        std::string app_name;
        std::string custom_data;
    };

    Buried(const std::string& work_path);
    ~Buried();

    BuriedResult Start(const Config& config);
    BuriedResult Report(std::string title, std::string data, uint32_t priority);


    std::shared_ptr<spdlog::logger> Logger();

private:
    void m_InitWorkPath(const std::string& work_path);
    void m_InitLogger();

    std::shared_ptr<spdlog::logger> m_logger;
    std::unique_ptr<buried::BuriedReport> m_buried_report;

    std::filesystem::path m_work_path;
};