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
    static void Trace(const std::string& fmt_str, Args&&... args)
    {
        spdlog::trace(
            fmt::runtime(fmt_str),
            std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Debug(const std::string& fmt_str, Args&&... args)
    {
        spdlog::debug(
            fmt::runtime(fmt_str),
            std::forward<Args>(args)...);
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
    static void Critical(const std::string& fmt_str, Args&&... args)
    {
        spdlog::critical(
            fmt::runtime(fmt_str),
            std::forward<Args>(args)...);
    }
};