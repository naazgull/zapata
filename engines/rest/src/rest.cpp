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

#include <zapata/rest/rest.h>

auto zpt::REST_RESOLVER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::rest::resolver_t::resolver_t(zpt::json _rest_config)
  : __configuration{ _rest_config } {}

auto zpt::rest::resolver_t::resolve(zpt::message _received,
                                    zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    std::list<zpt::event> _return;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
    auto _to_search = std::string{ "/" } + zpt::ontology::to_str(_received->performative()) +
                      _received->resource()->string();
#pragma GCC diagnostic pop
    for (auto [_, __, _record] : this->__catalog.search(_to_search)) {
        auto _hash_code = _record("metadata")("callback")->integer();
        expect(static_cast<unsigned>(_hash_code) < this->__callbacks.size(),
               "Couldn't find callback for [" << _hash_code << "]("
                                              << _received->resource()->string() << ")");
        _return.push_back(this->__callbacks[_hash_code](_received, _initializer));
    }
    expect(_return.size() != 0,
           "Couldn't find callback for (" << _received->resource()->string() << ")");
    return _return;
}
