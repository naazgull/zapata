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
 * @file manip.h
 * @brief Email sending utilities.
 *
 * Provides a simple interface for sending emails via the system sendmail command.
 */

#pragma once

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>

namespace zpt {

/**
 * @brief Sends an email via the system sendmail command.
 * @param _to Recipient email address.
 * @param _from Sender email address.
 * @param _subject Email subject line.
 * @param _message Email body content.
 * @param _replyto Optional Reply-To address.
 * @return True if sendmail command succeeded, false otherwise.
 *
 * @note Requires the sendmail command to be available on the system.
 *
 * @par Example Usage
 * @code
 * zpt::sendmail("user@example.com", "app@example.com",
 *               "Welcome", "Thanks for signing up!");
 * @endcode
 */
bool sendmail(std::string const& _to,
              std::string _from,
              std::string _subject,
              std::string _message,
              std::string _replyto = "");

} // namespace zpt
