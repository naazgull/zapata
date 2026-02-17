#include <iostream>
#include <zapata/rest.h>

namespace zpt {
namespace rest {
class minion_boot : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~minion_boot() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

class minion_shutdown : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~minion_shutdown() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

class minion_hello : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~minion_hello() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

class services_list : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~services_list() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

namespace services {
auto broadcast(std::string const& _path, zpt::json const& _config) -> void;
}
} // namespace rest
} // namespace zpt
