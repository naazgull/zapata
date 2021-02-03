#include <zapata/fsm.h>
#include <random>

auto
main(int argc, char* argv[]) -> int {
    zpt::fsm::machine<int, std::string> _sm{ 2,
                                             { "transitions",
                                               {
                                                 zpt::array,
                                                 { zpt::array, 0, { zpt::array, 1 } },    //
                                                 { zpt::array, 1, { zpt::array, 2, 3 } }, //
                                                 { zpt::array, 2, { zpt::array, 1, 3 } }     //
                                               },
                                               "begin",
                                               0,
                                               "undefined",
                                               -1 } };
    _sm->add_transition(0, [](int& _current, std::string& _data) -> int {
        std::ostringstream oss;
        oss << " -> S(" << _current << ")" << std::flush;
        _data.append(oss.str());
        return 1;
    });
    _sm->add_transition(1, [](int& _current, std::string& _data) -> int {
        std::ostringstream oss;
        oss << " -> S(" << _current << ")" << std::flush;
        _data.append(oss.str());
        if (_data.find("p1") != std::string::npos) { return 2; }
        else {
            return 3;
        }
    });
    _sm->add_transition(2, [](int& _current, std::string& _data) -> int {
        std::ostringstream oss;
        oss << " -> S(" << _current << ")" << std::flush;
        _data.append(oss.str());

        std::random_device _rd;
        std::mt19937 _gen(_rd());
        std::uniform_int_distribution<> _distrib(1, 100);
        if (_distrib(_gen) % 3) {
            return 1;
        }
        
        return 3;
    });
    _sm->add_transition(3, [](int& _current, std::string& _data) -> int {
        _data.append(" -> o\n");
        std::cout << _data << std::flush;
        return -1;
    });

    unsigned long _c{ 0 };
    do {
        std::string _d{ "(N" + std::to_string(_c) + "/p" + std::string{ _c % 2 ? "1" : "2" } +
                        ") ·" };
        _sm->begin(_d);
        ++_c;
        std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 10 });
    } while (true);

    return 0;
}
