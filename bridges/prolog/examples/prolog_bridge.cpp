#include <iostream>
#include <zapata/prolog.h>

auto test_conversion(std::string const& _to_convert) -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE();

    zpt::prolog::term _prolog{ _to_convert };
    std::cout << "PROLOG: " << _prolog << std::endl;
    auto _json = _bridge.object_to_json(_prolog);
    std::cout << "JSON: " << _json << std::endl;
    _prolog = _bridge.json_to_object(_json);
    std::cout << "PROLOG: " << _prolog << std::endl;
    _json = _bridge.object_to_json(_prolog);
    std::cout << "JSON: " << _json << std::endl;
}

auto test_module() -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE();
    char const* _path = "/tmp/zapata_prolog_bridge_example.pl";
    std::ofstream _out(_path);
    _out << ":- dynamic(likes/2).\n"
         << "likes(mary, wine).\n"
         << "likes(john, beer).\n";
    _out.close();

    _bridge.add_module(_path);
}

auto test_call() -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE();
    for (size_t _try = 0; _try != 10; ++_try) {
        auto _result = _bridge.call(zpt::prolog::term{ "(likes(X, Y), (person:X, likes:Y))" });
        std::cout << _result << std::endl;
        expect(_bridge.call(zpt::prolog::term{
                 std::format("assertz(likes(person{}, \"beer n.{}\"))", _try, _try) }),
               "assertz didn't succeed");
    }
}

auto test_foreign_function() -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE();
    _bridge.call(zpt::prolog::term{ std::format(
      "use_foreign_library(\"{}/lib/libzapata_bridge_prolog_bindings\")", ZPT_INSTALL_PREFIX) });
    std::cout << _bridge.call(zpt::prolog::term{ "(zpt_config(X), X)" }) << std::endl;
}

auto main(int, char** _argv) -> int {
    zpt::PROLOG_BRIDGE(std::string{ const_cast<const char*>(_argv[0]) });

    test_conversion(
      R"(((headers:("Server":zapata, "Date":"2026-01-01"), uri:"/something"), body:from:client))");
    test_conversion(
      R"((person:(name:"susan meyer", drink:tea, misc:[1, 2, 3], lives:apartment), dog:(name:edge)))");
    test_module();
    test_call();
    test_foreign_function();

    return 0;
}
