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

#include <vector>
#include <string>

namespace zpt {
auto
copy(std::vector<std::string> const& _from, std::vector<std::string>& _to) -> void {
    _to.clear();
    for (auto _e : _from) {
        _to.push_back(_e);
    }
}
auto
copy(std::string const& _from, std::string& _to) -> void {
    _to.clear();
    for (auto _e : _from) {
        _to.push_back(_e);
    }
}
}

#include <zapata/globals.h>
zpt::globals::cached<std::vector<std::string>> _global;

auto
main(int argc, char* argv[]) -> int {
    _global->push_back("a");
    _global->push_back("b");
    _global->push_back("c");

    std::thread _thread1{ [&]() -> void {
        std::cout << "Thread1:" << std::endl << std::flush;
        for (auto _e : *_global) {
            std::cout << _e << std::endl << std::flush;
        }
    } };
    _thread1.detach();
    std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 1000000 });
    _global.invalidate();

    std::thread _thread2{ [&]() -> void {
        _global->push_back("d");
        std::cout << "Thread2:" << std::endl << std::flush;
        for (auto _e : *_global) {
            std::cout << _e << std::endl << std::flush;
        }
    } };
    _thread2.detach();

    std::thread _thread3{ [&]() -> void {
        _global->push_back("e");
        _global.invalidate();
        _global->push_back("f");
        std::cout << "Thread3:" << std::endl << std::flush;
        for (auto _e : *_global) {
            std::cout << _e << std::endl << std::flush;
        }
    } };
    _thread3.detach();
    std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 1000000 });

    std::thread _thread4{ [&]() -> void {
        _global->push_back("g");
        std::cout << "Thread4:" << std::endl << std::flush;
        for (auto _e : *_global) {
            std::cout << _e << std::endl << std::flush;
        }
    } };

    _thread4.join();
    return 0;
}
