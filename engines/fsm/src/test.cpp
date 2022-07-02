#include <zapata/fsm.h>
#include <random>

class my_machine : public zpt::fsm::machine<my_machine, int, std::string, size_t> {
  public:
    my_machine()
      : zpt::fsm::machine<my_machine, int, std::string, size_t>{
          1,
          2,
          { "begin", 0, "end", 3, "undefined", -1, "pause", -2 }
      } {}

    auto verify_allowed_transition(int _from, int _to) -> void {}
    auto verify_transition(int _current) -> void {}
};

auto
main(int argc, char* argv[]) -> int {
    my_machine _sm{};
    _sm //
      .add_allowed_transitions({
        zpt::array,
        { zpt::array, 0, { zpt::array, 1 } },    //
        { zpt::array, 1, { zpt::array, 2, 3 } }, //
        { zpt::array, 2, { zpt::array, 1, 3 } }, //
        { zpt::array, 3, { zpt::array, 1 } }     //
      })                                         //
      .add_transition(0,
                      [](int& _current, std::string& _data, size_t const& _id) -> int {
                          std::ostringstream oss;
                          oss << " -> S(" << _current << ")" << std::flush;
                          _data.append(oss.str());
                          return 1;
                      }) //
      .add_transition(1,
                      [](int& _current, std::string& _data, size_t const& _id) -> int {
                          std::ostringstream oss;
                          oss << " -> S(" << _current << ")" << std::flush;
                          _data.append(oss.str());
                          if (_data.find("p1") != std::string::npos) { return 2; }
                          else { return 3; }
                      }) //
      .add_transition(2,
                      [](int& _current, std::string& _data, size_t const& _id) -> int {
                          std::ostringstream oss;
                          oss << " -> S(" << _current << ")" << std::flush;
                          _data.append(oss.str());
                          std::random_device _rd;
                          std::mt19937 _gen(_rd());
                          std::uniform_int_distribution<> _distrib(1, 100);
                          if (_distrib(_gen) % 3) { return 1; }
                          return 3;
                      }) //
      .add_transition(3, [](int& _current, std::string& _data, size_t const& _id) -> int {
          _data.append(" -> o");
          std::cout << _data << std::endl << std::flush;
          return -1;
      });
    _sm.start_threads();

    size_t _c{ 0 };
    do {
        std::string _d{ "(N" + std::to_string(_c) + "/p" + std::string{ _c % 2 ? "1" : "2" } +
                        ") ·" };
        _sm.begin(_d, _c);
        ++_c;
        std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 10 });
    } while (true);

    return 0;
}
