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
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <zapata/events.h>
#include <zapata/json.h>

#define UPNP_RAW -7

namespace zpt {

class UPnP;
class UPnPPtr;

class UPnPPtr : public std::shared_ptr<zpt::UPnP> {
  public:
    UPnPPtr();
    UPnPPtr(zpt::json _options);
    virtual ~UPnPPtr();
};

namespace upnp {
typedef zpt::UPnPPtr broker;
}

class UPnP : public zpt::Channel {
  public:
    UPnP(zpt::json _options);
    virtual ~UPnP();

    virtual auto notify(std::string const& _search, std::string const& _location) -> void;
    virtual auto search(std::string const& _search) -> void;
    virtual auto listen() -> zpt::http::req;

    virtual auto recv() -> zpt::json;
    virtual auto send(zpt::performative _performative,
                      std::string const& _resource,
                      zpt::json _payload) -> zpt::json;
    virtual auto send(zpt::json _envelope) -> zpt::json;

    virtual auto id() -> std::string;
    virtual auto underlying() -> zpt::socketstream_ptr;
    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;
    virtual auto close() -> void;
    virtual auto available() -> bool;
    virtual auto is_reusable() -> bool;

  private:
    std::mutex __mtx_underlying;
    std::mutex __mtx_send;
    zpt::socketstream_ptr __underlying;
    zpt::socketstream_ptr __send;
};
} // namespace zpt
