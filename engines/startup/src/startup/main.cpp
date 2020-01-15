#include <zapata/startup.h>

auto
main(int _argc, char* _argv[]) -> int {
    try {
        zpt::json _parameters = zpt::parameters::parse(_argc,
                                                       _argv,
                                                       { "--conf-file",
                                                         { zpt::array, "optional", "multiple" },
                                                         "--conf-dir",
                                                         { zpt::array, "optional", "multiple" } });

        auto& _boot = zpt::globals::get<zpt::startup::engine, zpt::BOOT_ENGINE>();
        _boot
          .initialize(_parameters) //
          .start();
    }
    catch (zpt::failed_expectation& _e) {
        std::cout << _e.what() << std::endl << std::flush;
    }
    return 0;
}
