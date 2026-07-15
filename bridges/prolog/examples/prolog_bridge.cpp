#include <iostream>
#include <zapata/prolog.h>

auto main(int, char** _argv) -> int {
    zpt::PROLOG_BRIDGE(std::string{ const_cast<const char*>(_argv[0]) });
    zpt::prolog::term _clause;
    auto& _t = *_clause;
    expect(PL_chars_to_term("(person:(name:\"susan meyer\", drink:tea, misc:[1, 2, 3], "
                            "lives:apartment), dog:(name:edge)).",
                            _t),
           "couldn't parse clause");

    auto& _bridge = zpt::PROLOG_BRIDGE();

    auto _json = _bridge.to_json(_clause);
    std::cout << "JSON: " << _json << std::endl;
    _json = _bridge.to_json(_clause);
    std::cout << "JSON: " << _json << std::endl;

    auto _prolog = _bridge.to_object(_json);
    std::cout << "PROLOG: " << _prolog << std::endl;

    _json = _bridge.to_json(_prolog);
    std::cout << "JSON: " << _json << std::endl;

    return 0;
}
