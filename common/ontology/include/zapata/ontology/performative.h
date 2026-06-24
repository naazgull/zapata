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
 * @file performative.h
 * @brief HTTP-like method constants (performatives) for message handling.
 *
 * Defines protocol-agnostic request methods that map to HTTP verbs
 * and additional methods for pub/sub and discovery protocols.
 */

#pragma once

#include <string>

namespace zpt {

/** @brief Type alias for HTTP-like method/verb. */
using performative = unsigned short;
/** @brief Type alias for HTTP-like status code. */
using status = unsigned short;

/**
 * @name Standard HTTP Performatives
 * @{
 */
inline const zpt::performative Get = 0;     ///< HTTP GET - retrieve resource
inline const zpt::performative Put = 1;     ///< HTTP PUT - replace resource
inline const zpt::performative Post = 2;    ///< HTTP POST - create resource
inline const zpt::performative Delete = 3;  ///< HTTP DELETE - remove resource
inline const zpt::performative Head = 4;    ///< HTTP HEAD - metadata only
inline const zpt::performative Options = 5; ///< HTTP OPTIONS - capability query
inline const zpt::performative Patch = 6;   ///< HTTP PATCH - partial update
inline const zpt::performative Reply = 7;   ///< Response message
/** @} */

/**
 * @name Extended Performatives
 * @{
 */
inline const zpt::performative Msearch = 8;           ///< SSDP M-SEARCH - discovery
inline const zpt::performative Notify = 9;            ///< SSDP NOTIFY - announcement
inline const zpt::performative Trace = 10;            ///< HTTP TRACE - diagnostic
inline const zpt::performative Connect = 11;          ///< HTTP CONNECT - tunnel
inline const zpt::performative Subscribe = 12;        ///< Pub/sub subscription
inline const zpt::performative Inform = 13;           ///< Push notification
inline const zpt::performative Performative_end = 14; ///< Sentinel value
/** @} */

/**
 * @brief Ontology conversion utilities.
 */
namespace ontology {

/**
 * @brief Converts performative to string name.
 * @param _performative Performative value.
 * @return String name (e.g., "GET", "POST").
 */
auto to_str(zpt::performative _performative) -> std::string;

/**
 * @brief Converts string name to performative.
 * @param _performative String name (case-insensitive).
 * @return Corresponding performative value.
 */
auto from_str(std::string _performative) -> zpt::performative;

} // namespace ontology

} // namespace zpt
