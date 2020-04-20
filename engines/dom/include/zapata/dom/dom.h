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

#include <zapata/pipeline.h>

#define URI_PART_ANY "{([^/?]+)}"

namespace zpt {
namespace dom {

static inline const unsigned short Meta = 0;
static inline const unsigned short Attribute = 1;
static inline const unsigned short Element = 2;

class element {
  public:
    element(std::string _xpath = "",
            std::string _name = "",
            zpt::json _content = zpt::undefined,
            zpt::json _parent = zpt::undefined);
    element(zpt::dom::element const& _rhs);
    element(zpt::dom::element&& _rhs);
    virtual ~element() = default;

    auto operator=(zpt::dom::element const& _rhs) -> zpt::dom::element&;
    auto operator=(zpt::dom::element&& _rhs) -> zpt::dom::element&;

    auto xpath() -> std::string&;
    auto name() -> std::string&;
    auto content() -> zpt::json;
    auto parent() -> zpt::json;

  private:
    std::string __xpath{ "" };
    std::string __name{ "" };
    zpt::json __content;
    zpt::json __parent;
};

class engine : public zpt::pipeline::engine<zpt::dom::element> {
  public:
    engine(size_t _pipeline_size = 1, int _threads_per_stage = 1, long _max_pop_wait_micro = 500);
    virtual ~engine() override = default;

    auto add_listener(size_t _stage,
                      std::string _pattern,
                      std::function<void(zpt::pipeline::event<zpt::dom::element>&)> _callback)
      -> zpt::dom::engine&;

    auto traverse(zpt::json _document, std::string _path = "", zpt::json _parent = zpt::undefined)
      -> zpt::dom::engine&;

    static auto on_error(zpt::json _path,
                         zpt::pipeline::event<zpt::dom::element>& _event,
                         const char* _what,
                         const char* _description = nullptr,
                         int _error = 500) -> bool;
};

} // namespace dom
} // nanespace zpt
