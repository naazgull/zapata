#include <iostream>
#include <zapata/prolog.h>

auto test_conversion() -> void {
    auto& _bridge = zpt::PROLOG_BRIDGE();

    zpt::prolog::term _prolog{ "(person:(name:\"susan meyer\", drink:tea, misc:[1, 2, 3], "
                               "lives:apartment), dog:(name:edge))." };
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

auto main(int, char** _argv) -> int {
    zpt::PROLOG_BRIDGE(std::string{ const_cast<const char*>(_argv[0]) });

    test_conversion();
    test_module();
    test_call();

    return 0;
}
