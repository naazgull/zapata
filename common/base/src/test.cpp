/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <zapata/base.h>
#include <zapata/allocator.h>

class A {
  public:
    std::string __member{ "xpto xpto xpto xpto" };
};

auto main(int, char*[]) -> int {
    {
        zpt::mem::pool _pool{ 64 };
        for (size_t _idx = 0; _idx != 100; ++_idx) {
            std::shared_ptr<A> _ptr = std::allocate_shared<A>(zpt::allocator<A>{ _pool });
            std::cout << "Allocate object holding '" << _ptr->__member << "'" << std::endl;
        }
    }
    {
        try {
            zpt::mem::pool _pool{ sizeof(A) };
            for (size_t _idx = 0; _idx != 100; ++_idx) {
                std::shared_ptr<A> _ptr = std::allocate_shared<A>(zpt::allocator<A>{ _pool });
            }
        }
        catch (std::bad_alloc const&) {
            std::cout
              << sizeof(A)
              << " is not enough because the allocator object is stored in the control block"
              << std::endl;
        }
    }
    {
        zpt::mem::pool _pool{ 1024 * 1024 * 1024 };
        {
            std::vector<A, zpt::allocator<A>> _v{ zpt::allocator<A>{ _pool } };
            for (size_t _idx = 0; _idx != 100; ++_idx) {
                _v.emplace_back();
                std::cout << "Allocate object holding '" << _v[_idx].__member << "'" << std::endl;
            }
            std::cout << "Allocated " << _pool.allocated_size() << " bytes" << std::endl;
        }
        std::cout << "Remaining " << _pool.allocated_size() << " bytes" << std::endl;
    }
}
