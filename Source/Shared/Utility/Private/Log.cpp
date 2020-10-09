#include "Log.h"
#include <cstdio>
#include <cstdarg>
#include <atomic>
#include "Ignore.h"

namespace AM
{
const std::atomic<Uint32>* Log::currentTickPtr = nullptr;

void Log::registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Log::info(const char* expression, ...)
{
#if defined(ENABLE_DEBUG_INFO)
    // If the app hasn't registered a tick count, default to 0.
    Uint32 currentTick = 0;
    if (currentTickPtr != nullptr) {
        currentTick = *currentTickPtr;
    }

    std::va_list arg;
    va_start(arg, expression);

    std::printf(" %u: ", currentTick);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    va_end(arg);
#else
    // Avoid unused parameter warnings in release.
    ignore(expression);
#endif // ENABLE_DEBUG_INFO
}

void Log::error(const char* fileName, int line, const char* expression, ...)
{
#if defined(ENABLE_DEBUG_INFO)
    // If the app hasn't registered a tick count, default to 0.
    Uint32 currentTick = 0;
    if (currentTickPtr != nullptr) {
        currentTick = *currentTickPtr;
    }

    std::va_list arg;
    va_start(arg, expression);

    std::printf(" Error at file: %s, line: %d, during tick: %u\n", fileName,
                line, currentTick);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    va_end(arg);
#else
    // Avoid unused parameter warnings in release.
    ignore(fileName);
    ignore(line);
    ignore(expression);
#endif // ENABLE_DEBUG_INFO
}

} // namespace AM
