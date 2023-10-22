#include <zapata/allocator.h>

class A {
  public:
    std::string __member{ "xpto xpto xpto xpto" };
};

auto main(int, char*[]) -> int {
    zpt::mem::pool _pool{ sizeof(A) * 5 };

    for (size_t _idx = 0; _idx != 6; ++_idx) {
        auto ptr = std::allocate_shared<A>(zpt::allocator<A>{ _pool });
        zlog("Allocated " << sizeof(A) << " bytes for object holding '" << ptr->__member << "'",
             zpt::info);
    }
}
