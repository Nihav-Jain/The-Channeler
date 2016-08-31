#pragma once

#include "EngineMinimal.h"

//Analytics Log
DECLARE_LOG_CATEGORY_EXTERN(LogChannelerAnalytics, Log, All);

/**
 * Lazy log routines
 * Usage: LOG("log string..")
 */
#define EXPAND(x) x
#define LOG_1_ARG(a) UE_LOG(LogChannelerAnalytics, Log, TEXT(a))
#define LOG_2_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_3_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_4_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_5_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_6_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_7_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_8_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)
#define LOG_9_ARG(a, ...) UE_LOG(LogChannelerAnalytics, Log, TEXT(a), __VA_ARGS__)

#define LOG_CHOOSER(_1 ,_2, _3, _4, _5, _6, _7, _8, _9, LOG_ROUTINE_NAME, ...) LOG_ROUTINE_NAME

#define LOG(...) EXPAND(EXPAND(LOG_CHOOSER(__VA_ARGS__, LOG_9_ARG, LOG_8_ARG, LOG_7_ARG, \
        LOG_6_ARG, LOG_5_ARG, LOG_4_ARG, LOG_3_ARG, LOG_2_ARG, LOG_1_ARG))(__VA_ARGS__))