#include <core/logger.h>
// #include <core/asserts.h>
#include <platform/platform.h>

// HACK: Temporary, before we've got a platform layer
#include <stdio.h>
#include <stdarg.h>

#if HPLATFORM_AVR && USE_PROGMEM
    #include <avr/pgmspace.h>
#endif


static PlatformStateT* loggerPlatformState = NULL;


void hkReportAssertionFailure(const char* expression, const char* message, const char* file, u16 line) {
    hkLogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}


b8 hkInitLogging(PlatformStateT* platformState) {
    loggerPlatformState = platformState;
    return plInitLogging(platformState);
}

void hkStopLogging(PlatformStateT* platformState) {
    HTRACE("logger.c -> hkStopLogging(PlatformStateT*):void");
    plStopLogging(platformState);
    return;
}


#if HPLATFORM_AVR && USE_PROGMEM
    #define LOG_LEVEL_STRING_LENGTH 10
    static const char logLevelStrings[6][LOG_LEVEL_STRING_LENGTH] PROGMEM = {
        "[FATAL]: ",
        "[ERROR]: ",
        "[WARN]:  ",
        "[INFO]:  ",
        "[DEBUG]: ",
        "[TRACE]: "
    };
#else
    static const char* const logLevelStrings[6] = {
        "[FATAL]: ",
        "[ERROR]: ",
        "[WARN]:  ",
        "[INFO]:  ",
        "[DEBUG]: ",
        "[TRACE]: "
    }; 
#endif

void hkLogOutput(LogLevelT level, const char* message, ...) {
    if(message == NULL) return;
    if((i16)level < 0 || (i16)level >= MAX_LOG_LEVEL) level = LOG_LEVEL_INFO;

    b8 isError = level < LOG_LEVEL_WARN;
    #if HPLATFORM_AVR && USE_PROGMEM
        char currentLogLevel[LOG_LEVEL_STRING_LENGTH];
        strcpy_P(currentLogLevel, logLevelStrings[level]);
    #else
        const char* currentLogLevel = logLevelStrings[level];
    #endif

    plClearFlag(loggerPlatformState, PL_GENERAL_ERROR);
    char logBuffer[LOG_MSG_MAX_LEN];
    plSetMem(logBuffer, 0, sizeof(logBuffer));

    va_list argPointer;
    va_start(argPointer, message);
    i32 written = vsnprintf(logBuffer, LOG_MSG_MAX_LEN, message, argPointer);
    va_end(argPointer);
    
    if(written < 0) return;
    if(written >= LOG_MSG_MAX_LEN) plSetFlag(loggerPlatformState, PL_GENERAL_ERROR);

    char logMessage[LOG_MSG_MAX_LEN];
    snprintf(logMessage, sizeof(logMessage), "%s%s\n", currentLogLevel, logBuffer);
    
    if(isError) plConsoleWriteError(logMessage);
    else plConsoleWrite(logMessage);
    
    return;
}