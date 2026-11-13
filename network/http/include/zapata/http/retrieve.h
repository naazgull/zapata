#pragma once

#include <zapata/http.h>
#include <zapata/ontology/message.h>

namespace zpt {
namespace http {
/** @brief Sends an HTTP request message and returns the reply.

Opens a TCP socket, writes the request, and reads the response.
Supports both HTTP and HTTPS based on the message URI scheme.
@param _to_send The HTTP request message to send.
@return The HTTP reply message containing status and body.
*/
auto retrieve(zpt::message _to_send) -> zpt::message;
} // namespace http
} // namespace zpt
