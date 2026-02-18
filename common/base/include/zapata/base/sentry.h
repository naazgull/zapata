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
 * @file sentry.h
 * @brief RAII scope guard for automatic cleanup.
 *
 * Provides a simple scope guard that executes a callback when destroyed,
 * useful for ensuring cleanup code runs regardless of how a scope exits.
 */

#pragma once

#include <functional>

namespace zpt {

/**
 * @brief RAII scope guard that executes a callback on destruction.
 * @tparam T Callable type (defaults to std::function<void()>).
 *
 * The sentry class provides a mechanism to execute cleanup code when a scope
 * exits, regardless of whether it exits normally or via exception. The callback
 * is invoked in the destructor.
 *
 * @par Example Usage
 * @code
 * void process_file() {
 *     auto* file = open_file("data.txt");
 *     zpt::sentry cleanup([file]() { close_file(file); });
 *
 *     // Work with file...
 *     // close_file() is called automatically when scope exits
 * }
 * @endcode
 *
 * @note The callback is always invoked on destruction. There is no mechanism
 *       to dismiss the sentry.
 */
template<typename T = std::function<void()>>
class sentry {
  public:
    /**
     * @brief Constructs a sentry with the given callback.
     * @param _callback The callable to invoke on destruction.
     */
    sentry(T _callback);

    /**
     * @brief Destructor that invokes the stored callback.
     */
    virtual ~sentry();

  private:
    T __underlying; ///< The callback to invoke on destruction.
};

} // namespace zpt

template<typename T>
zpt::sentry<T>::sentry(T _callback)
  : __underlying{ _callback } {}

template<typename T>
zpt::sentry<T>::~sentry() {
    this->__underlying();
}
