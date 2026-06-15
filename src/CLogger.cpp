#include "CLogger.h"

void CLogger::Trace(const std::string& msg)
{
    spdlog::trace(msg);
}

void CLogger::Debug(const std::string& msg)
{
    spdlog::debug(msg);
}

void CLogger::Info(const std::string& msg)
{
    spdlog::info(msg);
}

void CLogger::Warn(const std::string& msg)
{
    spdlog::warn(msg);
}

void CLogger::Error(const std::string& msg)
{
    spdlog::error(msg);
}

void CLogger::Critical(const std::string& msg)
{
    spdlog::critical(msg);
}