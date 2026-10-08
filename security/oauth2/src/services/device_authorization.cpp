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

#include <zapata/oauth2/oauth2.h>
#include <zapata/oauth2/services/device_authorization.h>

auto zpt::auth::oauth2::device_authorization::blocked() const -> bool { return false; }

auto zpt::auth::oauth2::device_authorization::operator()(zpt::events::dispatcher::ptr)
  -> zpt::events::state {
    auto _reply = zpt::OAUTH2_SERVER().device_authorization(this->received());
    auto _to_send = this->to_send();

    _to_send //
      ->status(_reply->status())
      .body() = _reply->body();
    _to_send->headers() |= _reply->headers();

    return zpt::events::finish;
}
