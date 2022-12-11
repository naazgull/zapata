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

#pragma once

#include <cstring>
#include <ctime>
#include <memory>
#include <iostream>
#include <sstream>
#include <zapata/base/performative.h>
#include <zapata/exceptions/ExpectationException.h>

/**
 * Compact form for throwing exceptions when validating logical requirements and
 * input/output
 * validation
 * @param x a boolean expression to be validated
 * @param y the error message
 * @param z the HTTP status code to be replied to the invoking HTTP client
 */
#define expect(x, y)                                                                               \
    if (!(x)) {                                                                                    \
        std::ostringstream __OSS__;                                                                \
        __OSS__ << y << std::flush;                                                                \
        throw zpt::ExpectationException(__OSS__.str(), #x, __LINE__, __FILE__);                    \
    }

namespace zpt {
auto get_tz() -> std::string const&;

using tm_ptr = std::shared_ptr<std::tm>;

auto get_time(time_t _t) -> zpt::tm_ptr;

} // namespace zpt
