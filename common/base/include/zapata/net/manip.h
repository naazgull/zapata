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
 * @brief Network utility functions.
 *
 * Provides utilities for querying network interface information.
 */

#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <ifaddrs.h>
#include <map>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <vector>

namespace zpt {
using addresses_t = std::map<std::string, std::vector<std::string>>;

namespace net {
/**
 * @brief Returns the IP address of a network interface.
 * @param _if Interface name (e.g., "eth0", "wlan0"). If empty, returns
 *        the first non-loopback interface's address.
 * @return IP address as a string, or empty string if not found.
 *
 * @par Example Usage
 * @code
 * std::string ip = zpt::net::getip("eth0");
 * std::string any_ip = zpt::net::getip();  // First available
 * @endcode
 */
auto getip(std::string const& _if = "") -> std::string;
/**
 * @brief Finds an available TCP or UDP port number.
 * @param _protocol Protocol name ("tcp" or "udp").
 * @param _start_from Port number to start searching from (default 1024).
 * @return An available port number, or 0 if none found.
 *
 * Scans sequentially from _start_from looking for a port that is not
 * in use by the system.
 */
auto get_available_port(std::string const& _protocol, std::uint32_t _start_from = 1024)
  -> std::uint32_t;
/** @brief Resolves a domain name to IP addresses. Performs a DNS lookup and returns IPv4 and IPv6
 * addresses found.
 * @param _address The domain name to resolve.
 * @return A map with "ipv4" and "ipv6" vectors of IP address strings.
 */
auto resolve_address(std::string const& _address) -> zpt::addresses_t;
/** @brief Tests whether a TCP connection can be established to the given address and port.
 * Resolves the address (IPv4 and IPv6) and attempts a connect to each resolved address until one
 * succeeds or all fail.
 * @param _address The domain name or IP address to probe.
 * @param _port The TCP port to probe.
 * @param _timeout The connection timeout in seconds (default 1).
 * @return True if a connection was established within the timeout, false otherwise.
 */
auto ping(std::string const& _address, std::uint16_t _port, std::uint32_t _timeout = 1) -> bool;
} // namespace net
} // namespace zpt
