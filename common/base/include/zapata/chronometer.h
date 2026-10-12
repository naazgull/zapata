#pragma once

#include <chrono>
#include <zapata/base/expect.h>

namespace zpt {
template<typename Unit, typename Ticks = std::uint64_t>
class chronometer {
  public:
    chronometer(bool _start = true);
    ~chronometer() = default;

    auto start() -> Ticks;
    auto reset() -> Ticks;
    auto lap() -> Ticks;
    auto get_elapsed_time() const -> Ticks;
    auto has_started() const -> bool;

  private:
    Ticks __start{ 0 };
};
} // namespace zpt

template<typename U, typename T>
zpt::chronometer<U, T>::chronometer(bool _start) {
    if (_start) { this->start(); }
}

template<typename U, typename T>
auto zpt::chronometer<U, T>::start() -> T {
    this->__start =
      std::chrono::duration_cast<U>(std::chrono::steady_clock::now().time_since_epoch()).count();
    return this->__start;
}

template<typename U, typename T>
auto zpt::chronometer<U, T>::reset() -> T {
    auto _previous = this->__start;
    this->__start = 0;
    return _previous;
}

template<typename U, typename T>
auto zpt::chronometer<U, T>::lap() -> T {
    expect(this->__start != 0, "The chronometer hasn't been started yet");
    auto _lap =
      std::chrono::duration_cast<U>(std::chrono::steady_clock::now().time_since_epoch()).count();
    auto _result = _lap - this->__start;
    this->__start = _lap;
    return _result;
}

template<typename U, typename T>
auto zpt::chronometer<U, T>::get_elapsed_time() const -> T {
    expect(this->__start != 0, "The chronometer hasn't been started yet");
    auto _lap =
      std::chrono::duration_cast<U>(std::chrono::steady_clock::now().time_since_epoch()).count();
    return _lap - this->__start;
}

template<typename U, typename T>
auto zpt::chronometer<U, T>::has_started() const -> bool {
    return this->__start != 0;
}
