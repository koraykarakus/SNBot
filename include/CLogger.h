#pragma once

#include <string>
#include <utility>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

class CLogger
{
public:
    static void Trace(const std::string& msg);
    static void Debug(const std::string& msg);
    static void Info(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);
    static void Critical(const std::string& msg);

    template<typename... Args>
    static void Trace(fmt::format_string<Args...> fmt, Args&&... args)
    {
        spdlog::trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Debug(fmt::format_string<Args...> fmt, Args&&... args)
    {
        spdlog::debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Info(const std::string& fmt_str, Args&&... args)
    {
        spdlog::info(
            fmt::runtime(fmt_str),
            std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Warn(const std::string& fmt_str, Args&&... args)
    {
        spdlog::warn(
            fmt::runtime(fmt_str),
            std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Error(const std::string& fmt_str, Args&&... args)
    {
        spdlog::error(
            fmt::runtime(fmt_str),
            std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Critical(fmt::format_string<Args...> fmt, Args&&... args)
    {
        spdlog::critical(fmt, std::forward<Args>(args)...);
    }
};