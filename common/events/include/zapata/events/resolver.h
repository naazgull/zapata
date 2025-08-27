#pragma once

#include <list>
#include <zapata/ontology.h>
#include <zapata/events/dispatcher.h>

namespace zpt {
namespace events {
using initializer_t = std::function<void(zpt::event _event)>;
using resolver_callback = std::function<zpt::event(zpt::message, zpt::events::initializer_t)>;
class resolver_t {
  public:
    resolver_t() = default;
    virtual ~resolver_t() = default;

    template<zpt::events::Operation T>
    auto add(std::string const& _path, zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
    template<zpt::events::Operation T>
    auto add(zpt::performative _performative,
             std::string const& _path,
             zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
    virtual auto add(zpt::json const& _service_description) -> resolver_t& = 0;
    virtual auto add(zpt::message _sent, zpt::events::resolver_callback callback)
      -> resolver_t& = 0;
    virtual auto add(zpt::performative _performtive,
                     std::string const& _path,
                     zpt::json const& _metadata,
                     zpt::events::resolver_callback _callback) -> resolver_t& = 0;
    template<zpt::events::Operation T>
    auto remove(std::string const& _path) -> resolver_t&;
    template<zpt::events::Operation T>
    auto remove(zpt::performative _performative, std::string const& _path) -> resolver_t&;
    virtual auto remove(zpt::message _sent) -> resolver_t& = 0;
    virtual auto remove(zpt::performative _performtive, std::string const& _path)
      -> resolver_t& = 0;
    virtual auto resolve(zpt::message _received, initializer_t _initializer) const
      -> std::list<zpt::event> = 0;
    virtual auto search(std::string const& _path, std::string const& _provider_id = "") const
      -> zpt::json = 0;
    virtual auto list(std::string const& _provider_id = "") const -> zpt::json = 0;
    virtual auto register_provider(zpt::json const& _service_description) -> resolver_t& = 0;
    virtual auto unregister_provider(std::string const& _id) -> resolver_t& = 0;
    virtual auto get_provider(std::string const& _id) const -> zpt::json = 0;
    virtual auto clear() -> resolver_t& = 0;
};
using resolver = std::shared_ptr<resolver_t>;

template<zpt::events::Operation T>
auto make_callback(zpt::message _received, zpt::events::initializer_t _initializer) -> zpt::event;
} // namespace events
} // namespace zpt

template<zpt::events::Operation T>
auto zpt::events::make_callback(zpt::message _received, zpt::events::initializer_t _initializer)
  -> zpt::event {
    auto _event = zpt::make_event<T>(_received);
    _initializer(_event);
    return _event;
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::add(std::string const& _path, zpt::json const& _metadata)
  -> resolver_t& {
    return this->add<T>(zpt::Performative_end, _path, _metadata);
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::add(zpt::performative _performative,
                                  std::string const& _path,
                                  zpt::json const& _metadata) -> resolver_t& {
    return this->add(_performative, _path, _metadata, zpt::events::make_callback<T>);
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::remove(std::string const& _path) -> resolver_t& {
    return this->remove<T>(zpt::Performative_end, _path);
}

template<zpt::events::Operation T>
auto zpt::events::resolver_t::remove(zpt::performative _performative, std::string const& _path)
  -> resolver_t& {
    return this->remove(_performative, _path);
}
