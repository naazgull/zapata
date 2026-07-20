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
#include <memory>
#include <netinet/in.h>
#include <string>

namespace zpt {
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
auto get_available_port(std::string const& _protocol, std::uint32_t _start_from = 1024)
  -> std::uint32_t;
} // namespace net
} // namespace zpt
