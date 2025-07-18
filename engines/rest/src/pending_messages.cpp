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

#include <zapata/rest/pending_messages.h>

auto zpt::rest::pending_messages::push(zpt::message _sent,
                                       zpt::events::resolver_callback _reply_callback)
  -> pending_messages& {
    std::unique_lock _guard{ this->__pending_mutex };
    this->__pending.insert(
      std::make_pair(_sent->headers()("X-Conversation-ID")->integer(), _reply_callback));
    return (*this);
}

auto zpt::rest::pending_messages::pop(zpt::message _received) -> zpt::events::resolver_callback {
    std::unique_lock _guard{ this->__pending_mutex };
    auto _found = this->__pending.find(_received->headers()("X-Conversation-ID")->integer());
    expect(_found != this->__pending.end(), "No pending message found");
    auto _to_return = _found->second;
    this->__pending.erase(_found);
    return _to_return;
}

auto zpt::rest::pending_messages::clear() -> pending_messages& {
    this->__pending.clear();
    return (*this);
}
