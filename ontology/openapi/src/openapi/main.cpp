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

#include <zapata/openapi.h>

auto
main(int argc, char* argv[]) -> int {
    try {
        std::ifstream _ifs;
        _ifs.open(argv[1]);
        zpt::json _config;
        _ifs >> _config;

        zlog(_config, zpt::info);

        zpt::openapi::engine _oa{ _config["limits"] };
        for (auto [_idx, _key, _source] : _config["openapi"]) {
            _oa.add_source(_source);
        }
        _oa.traverse();
        _oa.shutdown();
    }
    catch (zpt::SyntaxErrorException const& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    return 0;
}
