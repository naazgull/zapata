#pragma once

#include <iostream>
#include <zapata/rest.h>

namespace zpt {
namespace keystore {
class key_document : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~key_document() = default;
    auto update_element() -> zpt::events::state;
    auto get_element() -> zpt::events::state;
    auto remove_element() -> zpt::events::state;
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state override;

  protected:
  private:
};
} // namespace keystore
} // namespace zpt
