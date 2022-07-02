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
#include <signal.h>
#include <unistd.h>
#include <csignal>

auto
deallocate(int _signal) -> void {
    auto& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
    _boot.exit();
}

auto
nostop(int _signal) -> void {
    zlog("Please, use `zpt --terminate " << zpt::log_pid << "`", zpt::notice);
}

auto
main(int _argc, char* _argv[]) -> int {
    std::signal(SIGUSR1, deallocate);
    std::signal(SIGINT, deallocate);
    zpt::json _parameter_setup{
        "--conf-file",
        { "options",
          { zpt::array, "optional", "multiple" },
          "type",
          "path",
          "description",
          "configuration file" },
        "--conf-dir",
        { "options",
          { zpt::array, "optional", "multiple" },
          "type",
          "path",
          "description",
          "configuration directory, all the files in it are assumed to be configuration "
          "files" },
        "--help",
        { "options",
          { zpt::array, "optional", "single" },
          "type",
          "void",
          "description",
          "show this help" },
        "--terminate",
        { "options",
          { zpt::array, "optional", "single" },
          "type",
          "number",
          "description",
          "PID for the `zpt` process to terminate" }
    };
    try {
        zpt::json _parameters = zpt::parameters::parse(_argc, _argv, _parameter_setup);

        zpt::log_pname = std::make_unique<std::string>(_argv[0]);
        zpt::log_pid = ::getpid();

        if (_parameters["--help"]->ok()) {
            std::cout << zpt::parameters::usage(_parameter_setup) << std::flush;
            return 0;
        }

        if (_parameters["--terminate"]->ok()) {
            kill(static_cast<int>(_parameters["--terminate"]), SIGUSR1);
            return 0;
        }

        auto& _boot = zpt::globals::alloc<zpt::startup::engine>(zpt::BOOT_ENGINE(), _parameters);
        auto _config = zpt::globals::get<zpt::json>(zpt::GLOBAL_CONFIG());
        zpt::globals::alloc<zpt::stream::polling>(
          zpt::STREAM_POLLING(),
          std::max(static_cast<int>(_config["limits"]["max_producer_threads"]), 6) + 1,
          std::max(static_cast<long>(_config["limits"]["max_queue_spin_sleep"]), 50000L));
        zpt::globals::alloc<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        _boot //
          .add_thread(
            []() -> void { zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING()).pool(); })
          .start();

        zpt::globals::dealloc<zpt::stream::polling>(zpt::STREAM_POLLING());
        zpt::globals::dealloc<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        zpt::globals::dealloc<zpt::startup::engine>(zpt::BOOT_ENGINE());
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << _e.what() << std::endl
                  << std::endl
                  << zpt::parameters::usage(_parameter_setup) << std::flush;
    }
    return 0;
}
