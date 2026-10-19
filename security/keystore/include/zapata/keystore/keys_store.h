#pragma once

#include <iostream>
#include <zapata/rest.h>

namespace zpt {
namespace keystore {
class keys_store : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~keys_store() = default;
    auto add_element() -> zpt::events::state;
    auto list_elements() -> zpt::events::state;
    auto remove_elements() -> zpt::events::state;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state override;

  protected:
  private:
};
} // namespace keystore
} // namespace zpt
