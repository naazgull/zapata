#include <zapata/allocator.h>

auto zpt::MEM_POOL() -> zpt::mem::pool& {
    static zpt::mem::pool _global;
    return _global;
}

zpt::mem::pool::pool()
  : __max_size{ 0 }
  , __allocated_size{ 0 } {}

zpt::mem::pool::~pool() {
    if (this->allocated_size() != 0) {
        zlog(this->allocated_size()
               << " bytes of memory still accessible upon memory pool disposal",
             zpt::warning);
    }
}

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

#ifdef ALLOCATOR_DEBUG_MODE
auto zpt::mem::start_tracking() -> void {
    std::unique_lock guard{ zpt::mem::__allocated_mutex };
    zpt::mem::__allocated.clear();
}

auto zpt::mem::print_still_allocated() -> void {
    std::shared_lock guard{ zpt::mem::__allocated_mutex };
    for (auto const& [_address, _class] : zpt::mem::__allocated) {
        std::cout << _class << " @ 0x" << std::hex << _address << std::dec << "\n";
    }
    std::cout << std::flush;
}

auto zpt::mem::store(void* _ptr, std::string const& _name) -> void {
    std::unique_lock _guard{ zpt::mem::__allocated_mutex };
    zpt::mem::__allocated.insert(
      std::make_pair(reinterpret_cast<std::uint64_t>(_ptr), zpt::demangle(_name)));
}

auto zpt::mem::remove(void* _ptr) -> void {
    std::unique_lock _guard{ zpt::mem::__allocated_mutex };
    zpt::mem::__allocated.erase(reinterpret_cast<std::uint64_t>(_ptr));
}

#endif
