#pragma once
#include <iostream>
#include <string>
#include <sstream>

namespace MediocreBONK::Utils
{
    enum class LogLevel
    {
        INFO,
        WARNING,
        ERROR_LOG
    };

    class Logger
    {
    public:
        static void log(LogLevel level, const std::string& message)
        {
            std::string prefix;
            switch (level)
            {
            case LogLevel::INFO:
                prefix = "[INFO] ";
                break;
            case LogLevel::WARNING:
                prefix = "[WARNING] ";
                break;
            case LogLevel::ERROR_LOG:
                prefix = "[ERROR] ";
                break;
            }

            std::cout << prefix << message << std::endl;
        }

        static void info(const std::string& message)
        {
            log(LogLevel::INFO, message);
        }

        static void warning(const std::string& message)
        {
            log(LogLevel::WARNING, message);
        }

        static void error(const std::string& message)
        {
            log(LogLevel::ERROR_LOG, message);
        }
    };
}
