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

#include <functional>
#include <iostream>
#include <libetpan/libetpan.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <zapata/json.h>

namespace zpt {

class SMTP;
class SMTPPtr;

class SMTPPtr : public std::shared_ptr<zpt::SMTP> {
  public:
    /** @brief Constructs an SMTPPtr wrapping a new SMTP instance. */
    SMTPPtr();
    /** @brief Destroys the SMTPPtr. */
    virtual ~SMTPPtr();
};

namespace smtp {
typedef zpt::SMTPPtr broker;
}

class SMTP {
  public:
    /** @brief Constructs an SMTP client with default port 0. */
    SMTP();
    /** @brief Destroys the SMTP client, freeing any resources. */
    virtual ~SMTP();

    /** @brief Sets the credentials for SMTP authentication.
     @param _user The username for authentication.
     @param _passwd The password for authentication.
     */
    virtual auto credentials(std::string const& _user, std::string const& _passwd) -> void;

    /** @brief Returns the configured username. */
    virtual auto user() -> std::string;
    /** @brief Returns the configured password. */
    virtual auto passwd() -> std::string;

    /** @brief Connects to an SMTP server using the given connection URI.

     Parses the URI to extract host, port, and credentials.
     The URI scheme can be "smtp", "smtp+ssl", "smtp+tls", "esmtp", "esmtp+ssl", or "esmtp+tls".
     @param _connection The SMTP connection URI.
     */
    virtual auto connect(std::string const& _connection) -> void;
    /** @brief Sends an email message through the connected SMTP server.

     Composes the MIME email and delivers it via SMTP.
     @param _e_mail A JSON object with "From", "To", "Subject", and "Body" fields.
     */
    virtual auto send(zpt::json _e_mail) -> void;

  private:
    /** @brief The full connection URI string. */
    std::string __connection;
    /** @brief Parsed URI components extracted from the connection string. */
    zpt::json __uri;
    /** @brief The authentication username. */
    std::string __user;
    /** @brief The authentication password. */
    std::string __passwd;
    /** @brief The SMTP server host. */
    std::string __host;
    /** @brief The SMTP server port number. */
    uint __port;
    /** @brief Parsed scheme type (e.g., "esmtp" or "smtp" with optional "+ssl"/"+tls"). */
    zpt::json __type;
    /** @brief Mutex protecting concurrent access to the SMTP instance. */
    std::mutex __mtx;

    /** @brief Opens a connection to the SMTP server using libetpan.

     Handles SSL/TLS negotiation, HELO/EHLO exchange, and authentication.
     @return A pointer to the mailsmtp session, or null on failure.
     @throws failed_expectation if connection or authentication fails.
     */
    auto open() -> mailsmtp*;
    /** @brief Closes and frees a mailsmtp session.
     @param _smtp The mailsmtp session to close.
     */
    auto close(mailsmtp* _smtp) -> void;
    /** @brief Composes a MIME email string from a JSON message object.

     Generates headers (Date, From, To, Subject, etc.) and multipart body.
     @param _e_mail The JSON email object.
     @return The complete MIME-formatted email string ready for transmission.
     */
    auto compose(zpt::json _e_mail) -> std::string;
};
} // namespace zpt
