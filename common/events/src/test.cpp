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

#include <zapata/events/dispatcher.h>

class my_operator {
  public:
    my_operator(std::string _str, int _i)
      : __str{ _str }
      , __i{ _i } {}

    auto blocked() const -> bool { return true; }

    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
        zlog("job1: " << this->__str << " " << this->__i, zpt::info);
        ++this->__i;
        return zpt::events::retrigger;
    }

  private:
    std::string __str;
    int __i;
};

class my_other_operator {
  public:
    my_other_operator(int _i)
      : __i{ _i } {}

    auto blocked() const -> bool { return true; }

    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
        zlog("job2: xpto " << this->__i, zpt::info);
        _dispatcher.trigger<my_other_operator>(this->__i + 1);
        return zpt::events::finish;
    }

  private:
    int __i;
};

auto
main(int argc, char* argv[]) -> int {
    zpt::events::dispatcher _dispatcher{ 10 };

    _dispatcher //
      .start_consumers()
      .trigger<my_operator>("some string", 1)
      .trigger<my_other_operator>(1);

    std::this_thread::sleep_for(std::chrono::duration<int>{ 10 });
    _dispatcher //
      .stop_consumers();

    zlog("Stopping for 2s and restarting threads", zpt::info);
    std::this_thread::sleep_for(std::chrono::duration<int>{ 2 });
    _dispatcher //
      .start_consumers();

    std::this_thread::sleep_for(std::chrono::duration<int>{ 10 });
}
