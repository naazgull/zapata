#include <iostream>
#include <zapata/prolog.h>

auto main(int, char** _argv) -> int {
    zpt::PROLOG_BRIDGE(std::string{ const_cast<const char*>(_argv[0]) });
    zpt::prolog::term _clause;
    auto& _t = *_clause;
    expect(PL_chars_to_term("person(likes(susan, tea, [1, 2, 3], (a, b, c)), lives(apartment)).", _t), "couldn't parse clause");

    auto _json = zpt::PROLOG_BRIDGE().to_json(_clause);
    std::cout << _json << std::endl;

    return 0;
}
