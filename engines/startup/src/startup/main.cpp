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

#include <zapata/startup.h>
#include <zapata/transport.h>

auto
main(int _argc, char* _argv[]) -> int {
    try {
        zpt::json _parameters = zpt::parameters::parse(_argc,
                                                       _argv,
                                                       { "--conf-file",
                                                         { zpt::array, "optional", "multiple" },
                                                         "--conf-dir",
                                                         { zpt::array, "optional", "multiple" } });

        zpt::log_pname = std::make_unique<std::string>(_argv[0]);
        zpt::log_pid = ::getpid();

        zpt::globals::alloc<zpt::stream::polling>(zpt::STREAM_POLLING(), 10, 10000);
        zpt::globals::alloc<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        auto& _boot = zpt::globals::alloc<zpt::startup::engine>(zpt::BOOT_ENGINE());
        _boot
          .initialize(_parameters) //
          .start();
        zpt::globals::dealloc<zpt::stream::polling>(zpt::STREAM_POLLING());
        zpt::globals::dealloc<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        zpt::globals::dealloc<zpt::startup::engine>(zpt::BOOT_ENGINE());
    }
    catch (zpt::failed_expectation& _e) {
        std::cout << _e.what() << std::endl << std::flush;
    }
    return 0;
}
