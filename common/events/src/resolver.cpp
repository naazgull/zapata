#include <zapata/events/resolver.h>

auto zpt::events::resolver_t::count() const -> size_t {
    return this->__registered_callbacks->load();
}
