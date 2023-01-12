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

#include <iostream>
#include <zapata/startup.h>
#include <zapata/rest.h>
#include <zapata/transport.h>

class collection : public zpt::events::process {
  public:
    collection(zpt::message _received)
      : zpt::events::process{ _received } {}
    virtual ~collection() = default;

    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
        this->to_send()->status(200);
        this->to_send()->body() = { "a", 1 };
        return zpt::events::finish;
    }
};

class logger : public zpt::events::process {
  public:
    logger(zpt::message _received)
      : zpt::events::process{ _received } {}
    virtual ~logger() = default;

    auto blocked() const -> bool { return false; }
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
        zlog(this->received(), zpt::info);
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    zlog("Registering event in REST resolver", zpt::info);
    auto _resolver = zpt::global_cast<zpt::rest::resolver>(zpt::REST_RESOLVER());
    _resolver->add<collection>(zpt::Get, "/collection/{}");
    // _resolver->add<logger>("/{}/{}");
}

extern "C" auto _zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("Stopping event processing", zpt::info);
}
