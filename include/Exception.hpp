#pragma once

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

enum ExceptionType : uint8_t
{
    INVALID = 0,
    INVALID_SOCKET = 1,
};

class Exception : public std::runtime_error
{
  public:
    explicit Exception(const std::string &message) : std::runtime_error(message), type_(ExceptionType::INVALID)
    {
        std::string exceptionMessage = "Message :: " + message + "\n";
        std::cerr << exceptionMessage;
    }

    Exception(ExceptionType type, const std::string &message) : std::runtime_error(message), type_(type)
    {
        std::string exceptionMessage =
            "ExceptionType :: " + ExceptionTypeToStr(type_) + "\nMessage :: " + message + "\n";
        std::cerr << exceptionMessage;
    }

    static std::string ExceptionTypeToStr(ExceptionType type)
    {
        switch (type)
        {
        case ExceptionType::INVALID:
            return "Invalid";
        case ExceptionType::INVALID_SOCKET:
            return "Invalid socket";
        default:
            return "Unknow";
        }
    }

  private:
    ExceptionType type_;
};
