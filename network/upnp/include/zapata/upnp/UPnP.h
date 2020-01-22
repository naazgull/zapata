/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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

    virtual auto notify(std::string _search, std::string _location) -> void;
    virtual auto search(std::string _search) -> void;
    virtual auto listen() -> zpt::http::req;

    virtual auto recv() -> zpt::json;
    virtual auto send(zpt::performative _performative, std::string _resource, zpt::json _payload)
      -> zpt::json;
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
