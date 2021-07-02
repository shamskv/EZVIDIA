#pragma once
#include"BaseLog.hpp"
#include"LogFilePolicy.hpp"

#define Log BaseLog<LogFilePolicy> // define the policy we will use
#define LOG(level) if(LogLevel::level > Log::globalLevel) ; else Log().getLogger(LogLevel::level) 