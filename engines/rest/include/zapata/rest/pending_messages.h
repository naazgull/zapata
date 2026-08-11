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
 * @file pending_messages.h
 * @brief Tracks pending request/response pairings in the REST engine.
 *
 * Maintains a thread-safe map of sent messages awaiting replies,
 * keyed by message correlation ID.
 */

#pragma once

#include <unordered_map>
#include <zapata/locks/spin_mutex.h>
#include <zapata/transport.h>
#include <zapata/transport/engine.h>

namespace zpt {
namespace rest {

/**
 * @brief Thread-safe store for pending request/response callbacks.
 *
 * When a message is sent that expects a reply, the callback is stored
 * here. When the reply arrives, the callback is retrieved and invoked.
 */
class pending_messages {
  public:
    /**
     * @brief Default constructor (empty pending messages store).
     * @return void (constructors implicitly initialize the object).
     */
    pending_messages() = default;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~pending_messages() = default;

    /**
     * @brief Stores a callback for a sent message.
     * @param _sent The outbound message to track.
     * @param _context The call context for the request.
     * @param _callback Callback to invoke when the reply arrives.
     * @return Reference to this pending_messages instance.
     */
    auto push(zpt::message _sent,
              zpt::call_context::ptr _context,
              zpt::events::resolver_callback _callback) -> pending_messages&;
    /**
     * @brief Retrieves and removes the callback for a received reply.
     * @param _received The incoming reply message.
     * @return Tuple of call context pointer and resolver callback.
     */
    auto pop(zpt::message _received)
      -> std::tuple<zpt::call_context::ptr, zpt::events::resolver_callback>;
    /**
     * @brief Removes all pending callbacks.
     * @return Reference to this pending_messages instance.
     */
    auto clear() -> pending_messages&;

  private:
    /** @brief Map of pending request callbacks, keyed by correlation ID. */
    std::unordered_map<std::string,
                       std::tuple<zpt::call_context::ptr, zpt::events::resolver_callback>>
      __pending;
    /** @brief Mutex protecting the pending messages map. */
    zpt::locks::spin_mutex __pending_mutex;
};
} // namespace rest
} // namespace zpt
