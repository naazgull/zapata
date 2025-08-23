#pragma once

#include <zapata/events/dispatcher.h>

namespace zpt {
namespace events {
using initializer_t = std::function<void(zpt::event _event)>;
using resolver_callback = std::function<zpt::event(zpt::message, zpt::events::initializer_t)>;
class resolver_t {
  public:
    resolver_t() = default;
    virtual ~resolver_t() = default;

    template<typename T>
    auto add(std::string const& _path, zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
    template<typename T>
    auto add(zpt::performative _performative,
             std::string const& _path,
             zpt::json const& _metadata = zpt::undefined) -> resolver_t&;
    template<typename T>
    auto remove(std::string const& _path) -> resolver_t&;
    template<typename T>
    auto remove(zpt::performative _performative, std::string const& _path) -> resolver_t&;
    virtual auto resolve(zpt::message _received,
                         initializer_t _initializer) const -> std::list<zpt::event> = 0;
    virtual auto search(std::string const& _path,
                        std::string const& _provider_id = "") const -> zpt::json = 0;
    virtual auto list(std::string const& _provider_id = "") const -> zpt::json = 0;
    virtual auto register_provider(zpt::json const& _service_description) -> resolver_t& = 0;
    virtual auto unregister_provider(std::string const& _id) -> resolver_t& = 0;
    virtual auto get_provider(std::string const& _id) const -> zpt::json = 0;

  protected:
    virtual auto add(zpt::message _sent,
                     zpt::events::resolver_callback callback) -> resolver_t& = 0;
    virtual auto add(zpt::performative _performtive,
                     std::string const& _path,
                     zpt::json const& _metadata,
                     zpt::events::resolver_callback _callback) -> resolver_t& = 0;
    virtual auto add(zpt::json const& _service_description) -> resolver_t& = 0;
    virtual auto remove(zpt::message _sent) -> resolver_t& = 0;
    virtual auto remove(zpt::performative _performtive,
                        std::string const& _path) -> resolver_t& = 0;
};
using resolver = std::shared_ptr<resolver_t>;
} // namespace events

template<typename T>
auto zpt::events::resolver_t::add(std::string const& _path,
                                  zpt::json const& _metadata) -> zpt::events::resolver_t& {
    return this->add<T>(zpt::Performative_end, _path, _metadata);
}

template<typename T>
auto zpt::events::resolver_t::add(zpt::performative _performative,
                                  std::string const& _path,
                                  zpt::json const& _metadata) -> zpt::events::resolver_t& {
    return this->add(_performative, _path, _metadata, zpt::transports::make_callback<T>);
}

template<typename T>
auto zpt::events::resolver_t::remove(std::string const& _path) -> zpt::events::resolver_t& {
    return this->remove<T>(zpt::Performative_end, _path);
}

template<typename T>
auto zpt::events::resolver_t::remove(zpt::performative _performative,
                                     std::string const& _path) -> zpt::events::resolver_t& {
    return this->remove(_performative, _path);
}
