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
#include <zapata/allocator/allocator.h>

#include <string>
#include <memory>

auto
main(int argc, char* argv[]) -> int {
    zpt::allocator<std::string> _alloc{ 128 };

    std::shared_ptr<std::string> _shr_ptr =
      std::allocate_shared<std::string, zpt::allocator<std::string>>(_alloc);
    _shr_ptr->assign("hello world");

    using traits_t = std::allocator_traits<decltype(_alloc)>;

    std::string* _ptr1 = traits_t::allocate(_alloc, 2);
    traits_t::construct(_alloc, _ptr1, "foo");
    traits_t::construct(_alloc, _ptr1 + 1, "bar");

    std::string* _ptr2 = traits_t::allocate(_alloc, 1);
    traits_t::construct(_alloc, _ptr2, "hello!");

    std::cout << _alloc << std::endl << std::endl;
    std::cout << _ptr1[0] << ' ' << _ptr1[1] << std::endl << std::endl;
    std::cout << *_ptr2 << std::endl << std::endl;

    std::string* _ptr3{ nullptr };
    do {
        try {
            _ptr3 = traits_t::allocate(_alloc, 1);
            traits_t::construct(_alloc, _ptr3, "hello!");
            break;
        }
        catch (std::bad_alloc const& _e) {
            traits_t::destroy(_alloc, _ptr2);
            traits_t::deallocate(_alloc, _ptr2, 1);
        }
    } while (true);
    std::cout << _alloc << std::endl << std::endl;

    traits_t::destroy(_alloc, _ptr1 + 1);
    traits_t::destroy(_alloc, _ptr1);
    traits_t::deallocate(_alloc, _ptr1, 2);
    traits_t::destroy(_alloc, _ptr3);
    traits_t::deallocate(_alloc, _ptr3, 1);
    std::cout << _alloc << std::endl << std::endl;
    return 0;
}
