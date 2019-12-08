#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>

#include <semaphore.h>
#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/events.h>

class event_engine : public zpt::events::dispatcher<event_engine, int, zpt::json> {
  public:
    event_engine()
      : zpt::events::dispatcher<event_engine, int, zpt::json>{ 5, 2 } {}
    virtual ~event_engine() = default;

    auto trapped(int _event, zpt::json _content) -> void {
        auto _it = this->__callbacks.find(_event);
        if (_it != this->__callbacks.end()) {
            auto [_, _callback] = *_it;
            _callback(_content);
        }
    }
    auto listen_to(int _event, std::function<void(zpt::json)> _callback) {
        this->__callbacks.insert(std::make_pair(_event, _callback));
    }

  private:
    std::map<int, std::function<void(zpt::json)>> __callbacks;
};

auto
main(int _argc, char* _argv[]) -> int {
    try {
        event_engine _ee;
        _ee.listen(0, [](zpt::json _content) -> void {
            static thread_local long _counter{ 0 };
            ++_counter;
            std::cout << std::this_thread::get_id() << " (" << _counter << ") : " << _content
                      << std::endl
                      << std::flush;
        });
        _ee.listen(1, [](zpt::json _content) -> void {
            static thread_local long _counter{ 0 };
            ++_counter;
            std::cout << std::this_thread::get_id() << " (" << _counter << ") : " << _content
                      << std::endl
                      << std::flush;
        });

        std::thread _producer1{ [&]() -> void {
            static thread_local bool _which{ false };
            do {
                _which = !_which;
                _ee.trigger(_which,
                            _which ? zpt::json{ zpt::array, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
                                   : zpt::json{ "a", 1, "b", "xxx", "c", 6 });
                std::this_thread::yield();
            } while (true);
        } };
        std::thread _producer2{ [&]() -> void {
            static thread_local bool _which{ false };
            do {
                _which = !_which;
                _ee.trigger(_which,
                            _which ? zpt::json{ zpt::array, "X", "Y", "Z" }
                                   : zpt::json{ "k", "@@@", "l", 10000 });
                std::this_thread::yield();
            } while (true);
        } };
        std::thread _consumer1{ [&]() -> void { _ee.loop(); } };
        std::thread _consumer2{ [&]() -> void { _ee.loop(); } };

        _producer1.join();
        _producer2.join();
        _consumer1.join();
        _consumer2.join();
    }
    catch (zpt::missed_expectation& _e) {
        std::cout << _e.what() << " | " << _e.description() << std::endl << std::flush;
        exit(-10);
    }

    return 0;
}
