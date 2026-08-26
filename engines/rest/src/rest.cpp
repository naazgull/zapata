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

#include <zapata/http.h>
#include <zapata/net/self.h>
#include <zapata/net/socket/socket_stream.h>
#include <zapata/rest/rest.h>

zpt::rest::resolver_t::resolver_t(zpt::json _global_config)
  : __catalog{ zpt::CATALOG() }
  , __configuration{ _global_config } {
    this->__catalog->add_provider(zpt::IDENTITY()("_id")->string(), zpt::IDENTITY());
}

auto zpt::rest::resolver_t::add(zpt::message _sent,
                                zpt::call_context::ptr _context,
                                zpt::events::resolver_callback _callback)
  -> zpt::rest::resolver_t& {
    this->__pending_requests.push(_sent, _context, _callback);
    return (*this);
}

auto zpt::rest::resolver_t::add(zpt::performative _performative,
                                zpt::json const& _id,
                                zpt::json const& _metadata,
                                size_t _callback_hash,
                                zpt::events::resolver_callback _callback)
  -> zpt::rest::resolver_t& {
    auto _path = _id->string();
    {
        std::unique_lock _guard{ this->__callbacks_mutex };
        auto [_, _inserted] = this->__callbacks.insert(std::make_pair(_callback_hash, _callback));
        this->__registered_callbacks->fetch_add(_inserted);
    }
    auto _to_add =
      std::format("/{}{}",
                  (_performative == zpt::Performative_end ? std::string{ "{}" }
                                                          : zpt::ontology::to_str(_performative)),
                  _path);
    this->__catalog->add(_to_add, _callback_hash, _metadata);
    return (*this);
}

auto zpt::rest::resolver_t::add(zpt::json const& _service_description) -> zpt::rest::resolver_t& {
    expect(_service_description("_id")->ok(),
           "Member `_id` must be a part of the service description");
    expect(_service_description("provider_id")->ok(),
           "Member `provider_id` must be a part of the service description");
    this->__catalog->add(_service_description("_id")->string(),
                         _service_description("provider_id")->string(),
                         0,
                         _service_description("metadata"));
    return (*this);
}

auto zpt::rest::resolver_t::remove(zpt::message _sent) -> zpt::rest::resolver_t& {
    try {
        this->__pending_requests.pop(_sent);
    }
    catch (...) {
    }
    return (*this);
}

auto zpt::rest::resolver_t::remove(zpt::performative _performative,
                                   zpt::json const& _id,
                                   size_t _callback_hash) -> zpt::rest::resolver_t& {
    auto _path = _id->string();
    auto _to_remove =
      std::format("/{}{}",
                  (_performative == zpt::Performative_end ? std::string{ "{}" }
                                                          : zpt::ontology::to_str(_performative)),
                  _path);
    this->__catalog->remove(_to_remove, _callback_hash);
    std::unique_lock _guard{ this->__callbacks_mutex };
    this->__registered_callbacks->fetch_sub(this->__callbacks.erase(_callback_hash));
    return (*this);
}

auto zpt::rest::resolver_t::resolve(zpt::message _received,
                                    zpt::events::initializer_t _initializer) const
  -> std::list<zpt::event> {
    std::list<zpt::event> _return;

    if (_received->performative() != zpt::Reply) {
        auto _to_search = std::format("/{}{}",
                                      zpt::ontology::to_str(_received->performative()),
                                      _received->resource()->string());
        for (auto&& [_, __, _record] : this->__catalog->resolve(_to_search)) {
            auto _hash_code = static_cast<unsigned long long>(_record("hash")->integer());

            std::shared_lock _guard{ this->__callbacks_mutex };
            auto _found = this->__callbacks.find(_hash_code);
            expect(_found != this->__callbacks.end(),
                   "Couldn't find callback for [" << _hash_code << "]("
                                                  << _received->resource()->string() << ")");
            _return.push_back(_found->second(_received, nullptr, _initializer));
        }
    }
    else {
        auto [_context, _callback] = this->__pending_requests.pop(_received);
        _return.push_back(_callback(_received, _context, _initializer));
    }
    expect(_return.size() != 0,
           "Couldn't find callback for (" << _received->resource()->string() << ")");
    return _return;
}

auto zpt::rest::resolver_t::search(zpt::json const& _id, std::string const& _provider_id) const
  -> zpt::json {
    auto _path = _id->string();
    return this->__catalog->search(_path, _provider_id);
}

auto zpt::rest::resolver_t::list(std::string const& _provider_id) const -> zpt::json {
    return this->__catalog->list(_provider_id);
}

auto zpt::rest::resolver_t::register_provider(zpt::json const& _provider)
  -> zpt::rest::resolver_t& {
    this->__catalog->add_provider(_provider("_id")->string(), _provider);
    return (*this);
}

auto zpt::rest::resolver_t::unregister_provider(std::string const& _id) -> zpt::rest::resolver_t& {
    this->__catalog->remove_provider(_id);
    return (*this);
}

auto zpt::rest::resolver_t::get_provider(std::string const& _provider_id) const -> zpt::json {
    return this->__catalog->get_provider(_provider_id);
}

auto zpt::rest::resolver_t::clear() -> zpt::rest::resolver_t& {
    this->__pending_requests.clear();
    return (*this);
}

auto zpt::REST_RESOLVER(zpt::json _config) -> zpt::events::resolver {
    static zpt::events::resolver _global = zpt::allocate_shared<zpt::rest::resolver_t>(_config);
    return _global;
}
