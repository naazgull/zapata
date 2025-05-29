#include <iostream>
#include <zapata/rest.h>

namespace zpt {
namespace rest {
class services : public zpt::events::process {
  public:
    services(zpt::message _received);
    ~services() = default;
    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};
} // namespace rest
} // namespace zpt
