// From https://sureshjoshi.com/mobile/cross-platform-mobile-logging-macro

#ifndef __X_LOGGING_H__
#define __X_LOGGING_H__

#ifdef APP_RELEASE

#define LOG_VERBOSE(...)
#define LOG_DEBUG(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)

#else


#if defined(ANDROID) || defined(__ANDROID__)

#include <android/log.h>

#define LOG_TAG "TAG"
#ifndef NDEBUG // Only expose other log values in debug
    #define LOG_VERBOSE(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
    #define LOG_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
    #define LOG_WARN(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#endif
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#elif __APPLE__

#include <CoreFoundation/CoreFoundation.h>

extern "C" {
    void NSLog(CFStringRef format, ...);
    void CLSLog(CFStringRef format, ...);
}

#ifdef DEBUG // Only expose other log values in debug
    #define LOG_VERBOSE(format, ...) NSLog(CFSTR(format), ##__VA_ARGS__)
    #define LOG_DEBUG(format, ...) NSLog(CFSTR(format), ##__VA_ARGS__)
    #define LOG_WARN(format, ...) NSLog(CFSTR(format), ##__VA_ARGS__)
    #define LOG_ERROR(format, ...) NSLog(CFSTR(format), ##__VA_ARGS__)
#else
    #define LOG_ERROR(format, ...) CLSLog(CFSTR(format), ##__VA_ARGS__)
#endif

#else // Non-mobile platform

#include <iostream>

#define LOG_VERBOSE(...) std::clog << __VA_ARGS__ << std::endl;
#define LOG_DEBUG(...) std::clog << __VA_ARGS__ << std::endl;
#define LOG_WARN(...) std::clog << __VA_ARGS__ << std::endl;
#define LOG_ERROR(...) std::cerr << __VA_ARGS__ << std::endl;

#endif

#endif // APP_RELEASE

#endif // __X_LOGGING_H__
