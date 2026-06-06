#include "Logger.h"

void Logger::Trace(const std::string& strMsg)
{
    spdlog::trace(strMsg);
}

void Logger::Debug(const std::string& strMsg)
{
    spdlog::debug(strMsg);
}

void Logger::Info(const std::string& strMsg)
{
    spdlog::info(strMsg);
}

void Logger::Warn(const std::string& strMsg)
{
    spdlog::warn(strMsg);
}

void Logger::Error(const std::string& strMsg)
{
    spdlog::error(strMsg);
}

void Logger::Critical(const std::string& strMsg)
{
    spdlog::critical(strMsg);
}