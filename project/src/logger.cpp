#include "logger.h"

Logger::Logger() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    mConsoleLogger_ = std::make_shared<spdlog::logger>("Sever", console_sink);
}

Logger& Logger::Instance() {
    static Logger logger;
    return logger;
}