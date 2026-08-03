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
/** @brief Resolves a domain name to IP addresses.

Performs a DNS lookup and returns IPv4 and IPv6 addresses found.
@param _domain The domain name to resolve.
@return A JSON object with "ipv4" and "ipv6" arrays of IP address strings.
*/
auto resolve(std::string const& _domain) -> zpt::json;
} // namespace http
} // namespace zpt
