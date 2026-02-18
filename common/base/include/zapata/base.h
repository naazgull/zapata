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
