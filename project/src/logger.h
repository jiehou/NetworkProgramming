#ifndef LOGGER_H
#define LOGGER_H
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#include <memory>

class Logger {
public:
   static Logger& Instance();
   // prevent copy/copy assignment
   Logger(const Logger&) = delete;
   Logger& operator=(const Logger&) = delete;
   // prevent move/move assignment
   Logger(Logger&&) = delete;
   Logger& operator=(Logger&&) = delete;

    template<typename... Args>
    void Info(const char* msg, const Args&... args) {
       mConsoleLogger_->info(msg, args...);
    }

    template<typename... Args>
    void Warn(const char* msg, const Args&... args) {
        mConsoleLogger_->warn(msg, args...);
    }

    template<typename... Args>
    void Error(const char* msg, const Args&... args) {
        mConsoleLogger_->error(msg, args...);
    }
private:
    Logger();
private:
    std::shared_ptr<spdlog::logger> mConsoleLogger_;
};

#define LOG_INFO(msg, ...)      Logger::Instance().Info(msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)      Logger::Instance().Warn(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)      Logger::Instance().Error(msg, ##__VA_ARGS__)
#endif