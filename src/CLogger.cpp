#include "CLogger.h"

void CLogger::Trace(const std::string& strMsg)
{
    spdlog::trace(strMsg);
}

void CLogger::Debug(const std::string& strMsg)
{
    spdlog::debug(strMsg);
}

void CLogger::Info(const std::string& strMsg)
{
    spdlog::info(strMsg);
}

void CLogger::Warn(const std::string& strMsg)
{
    spdlog::warn(strMsg);
}

void CLogger::Error(const std::string& strMsg)
{
    spdlog::error(strMsg);
}

void CLogger::Critical(const std::string& strMsg)
{
    spdlog::critical(strMsg);
}