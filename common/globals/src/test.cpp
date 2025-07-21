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
#include <zapata/globals.h>

auto main(int, char**) -> int {
    zpt::cached<std::vector<std::string>> _global;
    zpt::cached<std::vector<std::string>> _global2;
    _global->push_back("a");
    _global->push_back("b");
    _global->push_back("c");
    _global2->push_back("1");
    _global2->push_back("2");
    _global2->push_back("3");

    zpt::thread_local_variable<std::string> _thread_local{ "this is the default value" };

    std::thread _thread1{ [&]() -> void {
        std::cout << "Thread1:" << std::endl << std::flush;
        for (auto _e : *_global) { std::cout << _e << std::endl << std::flush; }
        for (auto _e : *_global2) { std::cout << _e << std::endl << std::flush; }
        std::cout << "_thread_local: " << *_thread_local << std::endl << std::flush;
        *_thread_local = "this is the thread1 value";
        std::cout << "_thread_local_1: " << *_thread_local << std::endl << std::flush;
        _thread_local.dispose_local_image();
    } };
    _thread1.detach();
    std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 1000000 });
    _global.commit();
    _global2.commit();

    std::thread _thread2{ [&]() -> void {
        _global->push_back("d");
        _global2->push_back("4");
        std::cout << "Thread2:" << std::endl << std::flush;
        for (auto _e : *_global) { std::cout << _e << std::endl << std::flush; }
        for (auto _e : *_global2) { std::cout << _e << std::endl << std::flush; }
        _thread_local->assign("this is the thread2 value");
        std::cout << "_thread_local_2: " << *_thread_local << std::endl << std::flush;
        _thread_local.dispose_local_image();
    } };
    _thread2.detach();

    std::thread _thread3{ [&]() -> void {
        _global->push_back("e");
        _global.commit();
        _global->push_back("f");
        _global2->push_back("5");
        _global2.commit();
        _global2->push_back("6");
        std::cout << "Thread3:" << std::endl << std::flush;
        for (auto _e : *_global) { std::cout << _e << std::endl << std::flush; }
        for (auto _e : *_global2) { std::cout << _e << std::endl << std::flush; }
        std::cout << "_thread_local_3: " << *_thread_local << std::endl << std::flush;
        _thread_local.dispose_local_image();
    } };
    _thread3.detach();
    std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 1000000 });

    std::thread _thread4{ [&]() -> void {
        _global->push_back("g");
        _global2->push_back("7");
        std::cout << "Thread4:" << std::endl << std::flush;
        for (auto _e : *_global) { std::cout << _e << std::endl << std::flush; }
        for (auto _e : *_global2) { std::cout << _e << std::endl << std::flush; }
        *_thread_local = "this is the thread4 value";
        std::cout << "_thread_local_4: " << *_thread_local << std::endl << std::flush;
        _thread_local.dispose_local_image();
    } };

    _thread4.join();
    return 0;
}
