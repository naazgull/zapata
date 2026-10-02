#include <csignal>
#include <signal.h>
#include <unistd.h>
#include <zapata/runtime.h>
#include <zapata/startup.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace {
/**
 * @brief Signal handler that triggers stream polling shutdown.
 * @param _signal The signal number received.
 *
 * Called when SIGUSR1, SIGINT, or SIGTERM is received. Triggers the stream
 * polling shutdown sequence, which causes the runtime to cleanly unwind.
 */
auto deallocate(int _signal) -> void;
auto on_stream_connect(zpt::uuid const& _stream_id, std::string const& _address) -> void;
auto on_stream_disconnect(zpt::uuid const& _stream_id, std::string const& _address) -> void;
} // namespace

auto zpt::runtime::initialize(int _argc, char** _argv, zpt::json const& _default_config)
  -> zpt::json {
    std::signal(SIGUSR1, ::deallocate);
    std::signal(SIGINT, ::deallocate);
    std::signal(SIGTERM, ::deallocate);
    zpt::json _parameter_setup{
        "--config",
        { "options",
          { json_array, "optional", "multiple" },
          "type",
          "string",
          "description",
          "configuration file" },
        "--conf-dir",
        { "options",
          { json_array, "optional", "multiple" },
          "type",
          "string",
          "description",
          "configuration directory, all the files in it are assumed to be configuration "
          "files" },
        "--help",
        { "options",
          { json_array, "optional", "single" },
          "type",
          "bool",
          "description",
          "Print this message" },
        "--print-config",
        { "options",
          { json_array, "optional", "single" },
          "type",
          "bool",
          "description",
          "Prints the processed configuration" },
        "--terminate",
        { "options",
          { json_array, "optional", "single" },
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
        return json_null;
    }

    if (_parameters("--terminate")->ok()) {
        kill(static_cast<int>(_parameters("--terminate")), SIGUSR1);
        return json_null;
    }

    zpt::parameters::verify(_parameters, _parameter_setup);

    auto _config = zpt::GLOBAL_CONFIG();
    zpt::log_lvl = 5;
    zpt::log_format = 0;
    _config += _default_config;
    zpt::startup::configuration::load(_parameters, _config);
    _config["self"]["cmd"] = std::string{ const_cast<char const*>(_argv[0]) };
    _config["self"]["args"] = _parameters;

    if (_parameters("--print-config")->ok()) {
        std::cout << _config << std::endl << std::flush;
        return json_null;
    }

    zpt::log_lvl = _config("log")("level")->ok() ? static_cast<int>(_config("log")("level")) : 7;
    zpt::log_format =
      _config("log")("format")->ok() ? static_cast<int>(_config("log")("format")) : 0;
    if (_config("log")("target")->ok()) {
        zpt::log_fd = new std::ofstream{ _config("log")("target")->string() };
    }
    auto _consumers = std::max(1LL,
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
    zlog("Started global event dispatcher (" << _consumers << " threads)", zpt::info);

    return _config;
}

auto zpt::runtime::run() -> void {
    auto _config = zpt::GLOBAL_CONFIG();

    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(zpt::system_event_type::BOOTING);

    zpt::STREAM_POLLING();
    zlog("Initialized stream polling", zpt::info);
    zpt::STREAM_POLLING() //
      ->register_stream_state_listener(zpt::stream_state::CONNECTED, ::on_stream_connect)
      .register_stream_state_listener(zpt::stream_state::DISCONNECTED, ::on_stream_disconnect);
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
    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(zpt::system_event_type::EXITING);
    zpt::DISPATCHER() //
      ->stop_consumers();
    zpt::events::dispatcher::join_threads();
    zlog("Stopped all dispatcher worker threads", zpt::info);
    zpt::STREAM_POLLING() //
      ->unregister_stream_state_listener(zpt::stream_state::CONNECTED, ::on_stream_connect)
      .unregister_stream_state_listener(zpt::stream_state::DISCONNECTED, ::on_stream_disconnect);
    zpt::STREAM_POLLING() //
      ->close();
    zlog("Unloaded stream polling service", zpt::info);
    zpt::BOOT() //
      .unload();
    zlog("Unloaded all plugins", zpt::notice);
    zpt::TRANSPORT_LAYER() //
      .clear();
    zlog("Unloaded transport layer", zpt::info);
    zlog("Server PID " << zpt::log_pid << " stopped, exiting now", zpt::notice);

    expect(zpt::SYSTEM_EVENTS_RESOLVER()->count() == 0,
           zpt::SYSTEM_EVENTS_RESOLVER()->count()
             << " callbacks still registered in SYSTEM EVENTS resolver, it usually leads to "
                "segmentation faults due to dynamic library unloading");
}

auto zpt::runtime::shutdown() -> void { zpt::STREAM_POLLING()->shutdown(); }

auto zpt::runtime::is_in_shutdown() -> bool { return zpt::STREAM_POLLING()->is_in_shutdown(); }

namespace {
auto deallocate(int) -> void { zpt::STREAM_POLLING()->shutdown(); }

auto on_stream_connect(zpt::uuid const& _stream_id, std::string const& _address) -> void {
    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(
        zpt::system_event_type::STREAM_OPENED,
        zpt::json{ "state", "opened", "_id", _stream_id.to_string(), "address", _address });
}

auto on_stream_disconnect(zpt::uuid const& _stream_id, std::string const& _address) -> void {
    zpt::DISPATCHER() //
      ->trigger<zpt::system_event>(
        zpt::system_event_type::STREAM_CLOSED,
        zpt::json{ "state", "closed", "_id", _stream_id.to_string(), "address", _address });
}
} // namespace
