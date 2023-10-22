#include <zapata/allocator.h>

zpt::mem::pool::pool(size_t _max_memory)
  : __max_size{ _max_memory }
  , __allocated_size{ 0 } {}

zpt::mem::pool::~pool() {}

auto zpt::mem::pool::allocate(size_t _n) -> pointer_type {
    while (true) {
        auto _current_size = this->__allocated_size->load(std::memory_order_acquire);
        auto _new_size = _current_size + _n;
        expect(this->__max_size->load() == 0 || _new_size <= this->__max_size->load(),
               "reached max memory limit of " << this->__max_size << " bytes");
        if (this->__allocated_size->compare_exchange_strong(
              _current_size, _new_size, std::memory_order_release)) {
            break;
        }
    }
    return malloc(_n);
}

auto zpt::mem::pool::deallocate(pointer_type _ptr, size_t _n) -> void {
    this->__allocated_size->fetch_add(-_n);
    free(_ptr);
}

auto zpt::mem::pool::max_size() const -> size_t { return this->__max_size->load(); }

auto zpt::mem::pool::allocated_size() const -> size_t { return this->__allocated_size->load(); }
