#include <csignal>
#include <signal.h>
#include <unistd.h>
#include <zapata/runtime.h>
#include <zapata/startup.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace {
auto deallocate(int) -> void;
} // namespace

auto zpt::runtime::initialize(int _argc, char** _argv) -> void {
    std::signal(SIGUSR1, ::deallocate);
    std::signal(SIGINT, ::deallocate);
    std::signal(SIGTERM, ::deallocate);
    zpt::json _parameter_setup{
        "--config",
        { "options",
          { zpt::array, "optional", "multiple" },
          "type",
          "string",
          "description",
          "configuration file" },
        "--conf-dir",
        { "options",
          { zpt::array, "optional", "multiple" },
          "type",
          "string",
          "description",
          "configuration directory, all the files in it are assumed to be configuration "
          "files" },
        "--terminate",
        { "options",
          { zpt::array, "optional", "single" },
          "type",
          "int",
          "description",
          "PID for the `zpt` process to terminate" }
    };
    zpt::json _parameters = zpt::parameters::parse(_argc, _argv, _parameter_setup);

    zpt::log_pname = std::make_unique<std::string>(_argv[0]);
    zpt::log_pid = ::getpid();

    if (_parameters("--help")->ok()) {
        std::cout << zpt::parameters::usage(_parameter_setup) << std::flush;
        return;
    }

    if (_parameters("--terminate")->ok()) {
        kill(static_cast<int>(_parameters("--terminate")), SIGUSR1);
        return;
    }

    zpt::parameters::verify(_parameters, _parameter_setup);

    auto _config = zpt::GLOBAL_CONFIG();
    zpt::log_lvl = 8;
    zpt::log_format = 0;
    zpt::startup::configuration::load(_parameters, _config);

    zpt::log_lvl = _config("log")("level")->ok() ? static_cast<int>(_config("log")("level")) : 7;
    zpt::log_format =
      _config("log")("format")->ok() ? static_cast<int>(_config("log")("format")) : 0;
    if (_config("log")("target")->ok()) {
        zpt::log_fd = new std::ofstream{ _config("log")("target")->string() };
    }
    auto _consumers =
      std::max(1LL,
               _config("dispatcher")("limits")("max_workers")->ok()
                 ? _config("dispatcher")("limits")("max_workers")->integer()
                 : 1LL);
    zpt::MEM_POOL() //
      .max_size(_config("resources")("limits")("max_heap_allocation")->ok()
                  ? _config("resources")("limits")("max_heap_allocation")->integer()
                  : 0);

    zlog("Booting server PID " << zpt::log_pid, zpt::notice);
    zpt::DISPATCHER(_consumers, 10000) //
      ->start_consumers(_consumers);
    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(zpt::system_event_type::BOOTING);
    zlog("Started global event dispatcher (" << _consumers << " threads)", zpt::info);

    zpt::STREAM_POLLING();
    zlog("Initialized stream polling", zpt::info);
    zpt::TRANSPORT_LAYER(_config);
    zlog("Initialized transport layer", zpt::info);
    zpt::BOOT(_config) //
      .load();
    zlog("All plugins loaded", zpt::notice);

    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(zpt::system_event_type::FINISHED_BOOT);

    zpt::STREAM_POLLING() //
      ->poll();

    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(zpt::system_event_type::SHUTTING_DOWN);
    zpt::STREAM_POLLING() //
      ->shutdown();
    zlog("Unloaded stream polling service", zpt::info);
    zpt::BOOT() //
      .unload();
    zlog("Unloaded all plugins", zpt::notice);
    zpt::TRANSPORT_LAYER() //
      .clear();
    zlog("Unloaded transport layer", zpt::info);
    zpt::DISPATCHER() //
      ->stop_consumers();
    zlog("Stopped global event dispatcher", zpt::info);

    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(zpt::system_event_type::EXITING);
    zlog("Server PID " << zpt::log_pid << " stopped, exiting now", zpt::notice);
    if (_config("log")("target")->ok()) { delete zpt::log_fd; }
}

namespace {
auto deallocate(int) -> void { zpt::STREAM_POLLING()->shutdown(); }
} // namespace
