#include <core/logger.h>
#include <platform/platform.h>

// HACK: Temporary, before we've got a platform layer
#include <stdio.h>
#include <stdarg.h>

#if HPLATFORM_AVR && HK_USE_PROGMEM
    #include <avr/pgmspace.h>
#endif


static PlatformStateT* loggerPlatformState = NULL;

b8 hkInitLogging(PlatformStateT* platformState) {
    loggerPlatformState = (PlatformStateT*)platformState;
    return plInitLogging(loggerPlatformState);
}

void hkStopLogging() {
    HTRACE("logger.c -> hkStopLogging(PlatformStateT*):void");
    HDEBUG("hkStopLogging(): Stopping logging subsystem.");
    HINFO("Logging subsytem has been stopped.");

    plStopLogging(loggerPlatformState);
    return;
}


#if HPLATFORM_AVR && HK_USE_PROGMEM
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
    // On RP2040/2350 it is stored in flash
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
    #if HPLATFORM_AVR && HK_USE_PROGMEM
        char currentLogLevel[LOG_LEVEL_STRING_LENGTH];
        strcpy_P(currentLogLevel, logLevelStrings[level]);
    #else
        const char* currentLogLevel = logLevelStrings[level];
    #endif

    plClearFlag(loggerPlatformState, PL_GENERAL_ERROR);
    char logBuffer[HK_LOG_MSG_MAX_LEN];
    plSetMem(logBuffer, 0, sizeof(logBuffer));

    va_list argPointer;
    va_start(argPointer, message);
    i32 written = vsnprintf(logBuffer, HK_LOG_MSG_MAX_LEN, message, argPointer);
    va_end(argPointer);
    
    if(written < 0) return;
    if(written >= HK_LOG_MSG_MAX_LEN) plSetFlag(loggerPlatformState, PL_GENERAL_ERROR);

    char logMessage[HK_LOG_MSG_MAX_LEN];
    snprintf(logMessage, sizeof(logMessage), "%s%s\n", currentLogLevel, logBuffer);
    
    if(isError) plConsoleWriteError(logMessage);
    else plConsoleWrite(logMessage);
    
    return;
}