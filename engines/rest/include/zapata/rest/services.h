#include <iostream>
#include <zapata/rest.h>

namespace zpt {
namespace rest {
class minion_boot : public zpt::events::process {
  public:
    minion_boot(zpt::message _received);
    ~minion_boot() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

class services_collection : public zpt::events::process {
  public:
    services_collection(zpt::message _received);
    ~services_collection() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};
} // namespace rest
} // namespace zpt
