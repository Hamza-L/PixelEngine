#include "PixelLogger.h"

Logger *Logger::oneAndOnlyInstance = nullptr;
int Logger::uniqueID = 0;
int Logger::severity = 0;
std::vector<std::string> Logger::logMessages{};
int ScopeTracker::indentTracker = 0;

static std::string severityEnumToString(Level severityLevel) {
    switch (severityLevel) {
    case Level::FATAL:
        return "FATAL";
    case Level::ERROR:
        return "ERROR";
    case Level::WARNING:
        return "WARNING";
    case Level::DEBUG:
        return "DEBUG";
    case Level::INFO:
        return "INFO";
    }
    return "";
}

void Logger::Log(Level messageSeverity, const char* fileName, const int lineNumber, const char* func, const char* fmt, ...) {
    std::string inputMessage = fmt;
    std::string formatedFileName = fileName;
    std::stringstream message;

    formatedFileName = formatedFileName.substr(formatedFileName.find_last_of('/') + 1);
    for( int i = 0 ; i < ScopeTracker::indentTracker; i++ )
    {
        message << "\t";
    }
    message <<  formatedFileName <<":" << lineNumber << " - " << func << "(...) : " << severityEnumToString(messageSeverity) << " - ";

    va_list args;
    va_start(args, fmt);

    char token;
    int arg1;
    float arg2;
    double arg3;
    char *arg4;

    while (*fmt != '\0') {
        if (*fmt == '%') {
            token = *(++fmt);
            switch (token) {
            case 'd':
                arg1 = va_arg(args, int);
                message << arg1;
                break;

            case '%':
                message << token;
                break;

            case 's':
                arg4 = va_arg(args, char *);
                message << arg4;
                break;

            case 'f':
                arg3 = va_arg(args, double);
                message << arg3;
                break;
            }
        } else {
            message.put(*fmt);
        }
        fmt++;
    }

    va_end(args);

    logMessages.push_back(message.str());

    if (messageSeverity <= severity) {
        std::cout << message.str() << std::endl;
    }
}

void Logger::LogVar(Level messageSeverity, const char *fmt, ...) {
    std::string inputMessage = fmt;
    std::stringstream message;

    for( int i = 0 ; i < ScopeTracker::indentTracker; i++ )
    {
        message << "\t";
    }
    message << severityEnumToString(messageSeverity) << " - ";

    va_list args;
    va_start(args, fmt);

    char token;
    int arg1;
    float arg2;
    double arg3;
    char *arg4;

    while (*fmt != '\0') {
        if (*fmt == '%') {
            token = *(++fmt);
            switch (token) {
            case 'd':
                arg1 = va_arg(args, int);
                message << arg1;
                break;

            case '%':
                message << token;
                break;

            case 's':
                arg4 = va_arg(args, char *);
                message << arg4;
                break;

            case 'f':
                arg3 = va_arg(args, double);
                message << arg3;
                break;
            }
        } else {
            message.put(*fmt);
        }
        fmt++;
    }

    va_end(args);

    logMessages.push_back(message.str());

    if (messageSeverity <= severity) {
        std::cout << message.str() << std::endl;
    }
}
