/*
  Zapata project <https://github.com/naazgull/zapata>
  Author: n@zgul <n@zgul.me>

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file base.h
 * @brief Aggregate header for zapata-base module.
 *
 * Include this single header to access all base utilities:
 * - Assertion macros (expect)
 * - RAII scope guards (sentry)
 * - Cryptographic hashes (SHA-1, SHA-256, SHA-512)
 * - Logging facilities
 * - Text manipulation and encoding (Base64, UTF-8, URL)
 * - HTML entity encoding
 * - File system utilities (globbing)
 * - Network utilities (IP address lookup)
 * - Memory usage monitoring
 * - Email sending
 * - Spin-lock mutex
 * - Cache-aligned atomics
 *
 * @par Example Usage
 * @code
 * #include <zapata/base.h>
 *
 * int main() {
 *     zlog("Application starting", zpt::info);
 *
 *     std::string hash = zpt::crypto::sha256("data");
 *     std::string encoded = zpt::base64::r_encode("hello");
 *
 *     expect(hash.size() == 64, "SHA-256 produces 64 hex chars");
 *     return 0;
 * }
 * @endcode
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <utility>
#include <zapata/atomics/padded_atomic.h>
#include <zapata/base/expect.h>
#include <zapata/base/sentry.h>
#include <zapata/crypto/sha1.h>
#include <zapata/crypto/sha256.h>
#include <zapata/crypto/sha512.h>
#include <zapata/file/manip.h>
#include <zapata/locks/spin_mutex.h>
#include <zapata/log/log.h>
#include <zapata/mail/manip.h>
#include <zapata/mem/usage.h>
#include <zapata/net/manip.h>
#include <zapata/text/convert.h>
#include <zapata/text/html.h>
#include <zapata/text/manip.h>

/**
 * @brief Generates a `has_<m>()` trait function that checks for a zero-argument method `m`.
 * @param m Method name to check.
 * @return bool Trait function returning whether type `T` has the method `m`.
 */
#define has_method(m)                                                                              \
    template<class T>                                                                              \
    constexpr auto has_##m() -> bool {                                                             \
        constexpr bool has = requires(T& t) { t.m(); };                                            \
        if constexpr (has)                                                                         \
            return true;                                                                           \
        else                                                                                       \
            return false;                                                                          \
    }

/**
 * @brief Generates a `has_<m>()` trait function that checks for a one-argument method `m(C1)`.
 * @param m  Method name to check.
 * @param C1 Type of the first argument.
 * @return bool Trait function returning whether type `T` has the method `m(C1)`.
 */
#define has_method_1(m, C1)                                                                        \
    template<class T>                                                                              \
    constexpr auto has_##m() -> bool {                                                             \
        constexpr bool has = requires(T& t, C1 p1) { t.m(p1); };                                   \
        if constexpr (has)                                                                         \
            return true;                                                                           \
        else                                                                                       \
            return false;                                                                          \
    }

/**
 * @brief Generates a `has_<m>()` trait function that checks for a two-argument method `m(C1, C2)`.
 * @param m  Method name to check.
 * @param C1 Type of the first argument.
 * @param C2 Type of the second argument.
 * @return bool Trait function returning whether type `T` has the method `m(C1, C2)`.
 */
#define has_method_2(m, C1, C2)                                                                    \
    template<class T>                                                                              \
    constexpr auto has_##m() -> bool {                                                             \
        constexpr bool has = requires(T& t, C1 p1, C2 p2) { t.m(p1, p2); };                        \
        if constexpr (has)                                                                         \
            return true;                                                                           \
        else                                                                                       \
            return false;                                                                          \
    }

/**
 * @brief Generates a `has_<m>()` trait function that checks for a three-argument method
 *        `m(C1, C2, C3)`.
 * @param m  Method name to check.
 * @param C1 Type of the first argument.
 * @param C2 Type of the second argument.
 * @param C3 Type of the third argument.
 * @return bool Trait function returning whether type `T` has the method `m(C1, C2, C3)`.
 */
#define has_method_3(m, C1, C2, C3)                                                                \
    template<class T>                                                                              \
    constexpr auto has_##m() -> bool {                                                             \
        constexpr bool has = requires(T& t, C1 p1, C2 p2, C3 p3) { t.m(p1, p2, p3); };             \
        if constexpr (has)                                                                         \
            return true;                                                                           \
        else                                                                                       \
            return false;                                                                          \
    }
