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

#include <zapata/pipeline.h>

auto
main(int argc, char* argv[]) -> int {
    zpt::pipeline::engine<zpt::json> _engine{ 3, 2 };

    _engine.add_listener(0, "http:/{(.*)}", [](zpt::pipeline::event<zpt::json> _in) -> void {
        _in.set_path(zpt::r_replace(static_cast<std::string>(_in.path()["raw"]), "http:", ""));

        zpt::json _content = _in.content();
        std::cout << std::this_thread::get_id() << ": " << _content << std::endl << std::flush;
        _in.set_content({ "a", _content["k"], "b", _content["l"], "c", _content["m"] });
        _in.next_stage();
    });

    _engine.add_listener(1, "/users/{([^/]+)}", [](zpt::pipeline::event<zpt::json> _in) -> void {
        zpt::json _content = _in.content();
        std::cout << std::this_thread::get_id() << ": " << _content << std::endl << std::flush;
        _in.set_content({ "x", _content["a"], "z", _content["b"], "y", _content["c"] });
        _in.next_stage();
    });

    _engine.add_listener(2, "/users/{([^/]+)}", [](zpt::pipeline::event<zpt::json> _in) -> void {
        std::cout << std::this_thread::get_id() << ": " << _in.content() << std::endl << std::flush;
    });

    _engine.start_threads();

    unsigned long _c{ 0 };
    do {
        _engine.trigger("http:/users/1000", { "k", _c, "l", _c, "m", _c });
        ++_c;
        std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 1 });
    } while (true);

    // std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 3600000 });
    return 0;
}
