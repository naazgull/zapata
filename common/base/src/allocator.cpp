#include <zapata/allocator.h>

auto zpt::MEM_POOL() -> zpt::mem::pool& {
    static zpt::mem::pool _global;
    return _global;
}

zpt::mem::pool::pool()
  : __max_size{ 0 }
  , __allocated_size{ 0 } {}

zpt::mem::pool::~pool() {}

auto zpt::mem::pool::allocate(size_t _n) -> pointer_type {
    while (true) {
        auto _current_size = this->__allocated_size->load(std::memory_order_acquire);
        auto _new_size = _current_size + _n;
        if (this->__max_size->load() != 0 && _new_size >= this->__max_size->load()) {
            throw std::bad_alloc{};
        }
        if (this->__allocated_size->compare_exchange_strong(
              _current_size, _new_size, std::memory_order_release)) {
            break;
        }
    }
    return ::malloc(_n);
}

auto zpt::mem::pool::deallocate(pointer_type _ptr, size_t _n) -> void {
    this->__allocated_size->fetch_add(-_n);
    ::free(_ptr);
}

auto zpt::mem::pool::max_size(size_t _max_memory) -> pool& {
    if (_max_memory > this->max_size() && _max_memory > this->allocated_size()) {
        this->__max_size->store(_max_memory);
    }
    return (*this);
}

auto zpt::mem::pool::max_size() const -> size_t { return this->__max_size->load(); }

auto zpt::mem::pool::allocated_size() const -> size_t { return this->__allocated_size->load(); }

auto zpt::mem::pool::to_string() const -> std::string {
    return std::format(
      "{{ \"max\": {}, \"allocated\": {} }}", this->max_size(), this->allocated_size());
}
