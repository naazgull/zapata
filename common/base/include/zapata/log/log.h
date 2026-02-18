/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file log.h
 * @brief Logging facilities for the Zapata framework.
 *
 * Provides logging macros and functions with configurable log levels,
 * output destinations, and formatting. Also includes a thread sleep timer utility.
 */

#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

/** @brief Macro that expands to the current hostname. */
#define __HOST__ std::string(zpt::log_hostname())

/**
 * @def zlog(x, y)
 * @brief Logs a message at the specified level.
 * @param x The message (can be a stream expression).
 * @param y The log level (e.g., zpt::debug, zpt::info).
 *
 * Only logs if the specified level is less than or equal to zpt::log_lvl.
 * Automatically captures source file and line number.
 */
#define zlog(x, y)                                                                                 \
    if (y <= zpt::log_lvl) {                                                                       \
        std::ostringstream __OSS__;                                                                \
        __OSS__ << x << std::flush;                                                                \
        zpt::log(__OSS__.str(), y, __HOST__, __LINE__, __FILE__);                                  \
    }

/** @brief Logs a debug-level message. */
#define zdbg(x) zlog(x, zpt::debug)

/** @brief Logs a trace-level message. */
#define ztrace(x) zlog(x, zpt::trace)

/** @brief Logs a verbose-level message. */
#define zverbose(x) zlog(x, zpt::verbose)

namespace zpt {

extern short int log_lvl;                    ///< Current log level threshold.
extern std::ostream* log_fd;                 ///< Output stream for log messages.
extern long log_pid;                         ///< Process ID for log messages.
extern std::unique_ptr<std::string> log_pname; ///< Process name for log messages.
extern short log_format;                     ///< Log output format.
extern const char* log_lvl_names[];          ///< Human-readable log level names.

/**
 * @brief Log severity levels.
 *
 * Follows syslog-style severity levels, with additional trace and verbose
 * levels for fine-grained debugging.
 */
enum LogLevel {
    emergency = 0,  ///< System is unusable.
    alert = 1,      ///< Action must be taken immediately.
    critical = 2,   ///< Critical conditions.
    error = 3,      ///< Error conditions.
    warning = 4,    ///< Warning conditions.
    notice = 5,     ///< Normal but significant condition.
    info = 6,       ///< Informational messages.
    debug = 7,      ///< Debug-level messages.
    trace = 8,      ///< Fine-grained tracing.
    verbose = 9     ///< Most verbose output.
};

/**
 * @brief Writes a log message.
 * @param _text The message text.
 * @param _level The severity level.
 * @param _host The hostname.
 * @param _line The source line number.
 * @param _file The source file name.
 * @return Status code (0 on success).
 */
auto log(std::string const& _text,
         zpt::LogLevel _level,
         std::string const& _host,
         int _line,
         std::string const& _file) -> int;

/**
 * @brief Template overload for logging arbitrary types.
 * @tparam T Type that can be converted to string via to_string().
 * @param _text The value to log.
 * @param _level The severity level.
 * @param _host The hostname.
 * @param _line The source line number.
 * @param _file The source file name.
 * @return Status code (0 on success).
 */
template<typename T>
auto log(T _text,
         zpt::LogLevel _level,
         std::string const& _host,
         int _line,
         std::string const& _file) -> int {
    return zpt::log(to_string(_text), _level, _host, _line, _file);
}

/**
 * @brief Returns the current hostname.
 * @return The hostname string.
 */
auto log_hostname() -> std::string;

/**
 * @brief Thread utilities namespace.
 */
namespace this_thread {

/**
 * @brief Adaptive sleep timer with exponential backoff.
 * @tparam T Arithmetic type for time duration (e.g., double for seconds).
 *
 * Provides a sleep mechanism that starts with yielding (no sleep), then
 * gradually increases sleep duration up to a maximum. Useful for polling
 * loops that should be responsive initially but conserve CPU when idle.
 *
 * @par Example Usage
 * @code
 * zpt::this_thread::timer<double> backoff{0.001, 5}; // 1ms step, 5 non-waiting cycles
 * while (!done) {
 *     if (check_condition()) break;
 *     backoff.sleep_for(1.0);  // Sleep up to 1 second max
 * }
 * @endcode
 */
template<typename T>
class timer {
    static_assert(std::is_arithmetic<T>::value,
                  "Type `T` in `zpt::this_thread::timer` must be an arithmetic type.");

  public:
    /**
     * @brief Constructs a timer with the given step size.
     * @param steps The time increment per sleep_for() call.
     * @param _non_waiting_cycles Number of initial cycles that only yield.
     */
    timer(T steps, unsigned int _non_waiting_cycles = 0);
    virtual ~timer() = default;

    /**
     * @brief Resets the timer to its initial state.
     * @return Reference to this timer.
     */
    auto reset() -> zpt::this_thread::timer<T>&;

    /**
     * @brief Sleeps for an adaptive duration.
     * @param _upper_limit Maximum sleep duration.
     * @return The actual sleep duration used.
     *
     * Each call increases the sleep duration by the step size, up to
     * the upper limit. Initial calls may only yield without sleeping.
     */
    auto sleep_for(T _upper_limit) -> T;

  private:
    T __sleep_tics{ 0 };                ///< Current sleep duration.
    T __step{ 0 };                      ///< Duration increment per call.
    unsigned int __non_waiting_cycles{ 0 }; ///< Non-sleeping cycles remaining.
};

} // namespace this_thread
} // namespace zpt

template<typename T>
zpt::this_thread::timer<T>::timer(T _step, unsigned int _non_waiting_cycles)
  : __sleep_tics{ _step * -_non_waiting_cycles }
  , __step{ _step }
  , __non_waiting_cycles{ _non_waiting_cycles } {}

template<typename T>
auto zpt::this_thread::timer<T>::reset() -> zpt::this_thread::timer<T>& {
    this->__sleep_tics = -this->__non_waiting_cycles * this->__step;
    return (*this);
}

template<typename T>
auto zpt::this_thread::timer<T>::sleep_for(T _upper_limit) -> T {
    if (this->__sleep_tics > _upper_limit) { this->__sleep_tics = _upper_limit; }
    else { this->__sleep_tics += this->__step; }
    if (this->__sleep_tics <= 0) { std::this_thread::yield(); }
    else { std::this_thread::sleep_for(std::chrono::duration<T>{ this->__sleep_tics }); }
    return this->__sleep_tics;
}
