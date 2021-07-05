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

#pragma once

#include <zapata/startup.h>
#include <zapata/transport.h>
#include <zapata/pipeline.h>

#define URI_PART_ANY "{([^/?]+)}"

namespace zpt {
auto
REST_ENGINE() -> ssize_t&;
namespace rest {
class engine : public zpt::pipeline::engine<zpt::exchange> {
  public:
    engine(size_t _pipeline_size, zpt::json _configuration);
    virtual ~engine() = default;

    auto add_listener(std::string _pattern,
                      std::function<void(zpt::pipeline::event<zpt::exchange>&)> _callback)
      -> zpt::rest::engine&;
    auto add_listener(size_t _step,
                      std::string _pattern,
                      std::function<void(zpt::pipeline::event<zpt::exchange>&)> _callback)
      -> zpt::rest::engine&;
    auto request(std::string _uri,
                 std::function<void(zpt::pipeline::event<zpt::exchange>&)> _callback)
      -> zpt::rest::engine&;

    static auto on_error(zpt::json& _path,
                         zpt::pipeline::event<zpt::exchange>& _event,
                         const char* _what,
                         const char* _description = nullptr,
                         const char* _backtrace = nullptr,
                         int _error = -1,
                         int _status = 500) -> bool;

  private:
    zpt::lf::spin_lock __pending_lock;
    std::map<std::string, std::function<void(zpt::pipeline::event<zpt::exchange>&)>> __pending;
};

} // namespace rest
} // nanespace zpt
