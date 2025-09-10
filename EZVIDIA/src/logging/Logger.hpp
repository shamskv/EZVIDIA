#pragma once
#include "BaseLog.hpp"
#include "LogFilePolicy.hpp"

#define Logger BaseLog<LogFilePolicy>  // define the policy we will use
#define LOG(level)                            \
  if (LogLevel::level > Logger::loggingLevel) \
    ;                                         \
  else                                        \
    Logger().getLogger(LogLevel::level)