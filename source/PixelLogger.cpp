#include "PixelLogger.h"
#include <cstring>

Logger *Logger::oneAndOnlyInstance = nullptr;
int Logger::uniqueID = 0;
ErrorLevel Logger::severity = INFO;
std::vector<std::string> Logger::logMessages{};
int ScopeTracker::indentTracker = 1;

static std::string severityEnumToString(ErrorLevel severityLevel) {
    switch (severityLevel) {
    case ErrorLevel::FATAL:
        return "FATAL";
    case ErrorLevel::ERROR:
        return "ERROR";
    case ErrorLevel::WARNING:
        return "WARNING";
    case ErrorLevel::OK:
        return "OK";
    case ErrorLevel::DEBUG:
        return "DEBUG";
    case ErrorLevel::INFO:
        return "INFO";
    }
    return "";
}

ScopeTracker::~ScopeTracker() {

    ErrorLevel globalLevel = Logger::get_instance()->getSeverity();
        if (m_severity <= Logger::get_instance()->getSeverity()) {
            for (int i = 0; i < indentTracker; i++) {
                std::cout << "\t";
            }
            std::cout << "}" << std::endl;
        }
        indentTracker--;

    }

void Logger::Log(ErrorLevel messageSeverity, const char* fileName, const int lineNumber, const char* func, const char* fmt, ...) {
    std::string inputMessage = fmt;
    std::string formatedFileName = fileName;
    std::stringstream message;

    message << severityEnumToString(messageSeverity) << " - ";
    formatedFileName = formatedFileName.substr(formatedFileName.find_last_of('/') + 1);
    for( int i = 0 ; i < ScopeTracker::indentTracker; i++ )
    {
        message << "\t";
    }
    if(strlen(fileName)>0) message <<  formatedFileName << ":";
    if(lineNumber >= 0) message << lineNumber;
    if(strlen(func)>0) message << " - " << func << "(...)";

    if(std::strcmp(fmt, ""))
        message << "{\t//";

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

void Logger::LogVar(ErrorLevel messageSeverity, const char *fmt, ...) {
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
