/*
  Zapata project <https://github.com/naazgull/zapata>
  Author: n@zgul <n@zgul.me>

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <signal.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <sys/types.h>
#include <sys/ipc.h>
#include <zapata/lockfree/atomics.h>
#include <zapata/lockfree/queue.h>
#include <zapata/lockfree/list.h>
#include <zapata/log/log.h>
#include <zapata/text/manip.h>
#include <zapata/exceptions/NoMoreElementsException.h>

constexpr int N_ELEMENTS_QUEUE = 10000;
constexpr int MAX_THREADS_QUEUE = 1000;

constexpr int N_ELEMENTS_LIST = 1000;
constexpr int MAX_THREADS_LIST = 50;

constexpr int PER_THREAD = 2;

// #define QUEUE_USE_STRING
// #define INTERCEPT_SIGINT
#define SPIN_WAIT_MICROS 5

std::atomic<int> _pushed{ 0 };
std::atomic<int> _poped{ 0 };

#ifdef QUEUE_USE_STRING
zpt::lf::queue<std::shared_ptr<std::string>>::hazard_domain _q_hazard_domain{ MAX_THREADS_QUEUE,
                                                                              PER_THREAD };
zpt::lf::list<std::shared_ptr<std::string>>::hazard_domain _l_hazard_domain{ MAX_THREADS_LIST,
                                                                             PER_THREAD };
zpt::lf::queue<std::shared_ptr<std::string>> _queue{ _q_hazard_domain, SPIN_WAIT_MICROS };
zpt::lf::list<std::shared_ptr<std::string>> _list{ _l_hazard_domain };
#else
zpt::lf::queue<int>::hazard_domain _q_hazard_domain{ MAX_THREADS_QUEUE, PER_THREAD };
zpt::lf::list<int>::hazard_domain _l_hazard_domain{ MAX_THREADS_LIST, PER_THREAD };
zpt::lf::queue<int> _queue{ _q_hazard_domain, SPIN_WAIT_MICROS };
zpt::lf::list<int> _list{ _l_hazard_domain };
#endif

auto
pause(int _signal) -> void {
    std::cout << _queue << std::endl << std::flush;
    std::cout << "* " << MAX_THREADS_QUEUE << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;
}

auto
test_queue() -> int {
    _pushed = 0;
    _poped = 0;
    auto _t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> _threads;

#ifdef INTERCEPT_SIGINT
    std::signal(SIGINT, pause);
#endif

    for (int _i = 0; _i != MAX_THREADS_QUEUE; ++_i) {
        _threads.emplace_back(
          [&](int _n_thread) {
              if (_n_thread % 2 == 0) {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS_QUEUE; ++_k) {
#ifdef QUEUE_USE_STRING
                          std::shared_ptr<std::string> _value{ new std::string(
                            std::to_string(_n_thread * N_ELEMENTS_QUEUE + _k)) };
#else
                          int _value = _n_thread * N_ELEMENTS_QUEUE + _k + 1;
#endif
                          _queue.push(_value);
                          ++_pushed;
                      }
                  }
                  catch (zpt::failed_expectation const& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
              else {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS_QUEUE;) {
                          try {
                              _queue.pop();
                              ++_k;
                              ++_poped;
                          }
                          catch (zpt::NoMoreElementsException const& e) {
                              std::this_thread::yield();
                          }
                      }
                  }
                  catch (zpt::failed_expectation const& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
          },
          _i);
    }

    for (int _i = 0; _i != MAX_THREADS_QUEUE; ++_i)
        _threads[_i].join();
    auto _t2 = std::chrono::high_resolution_clock::now();
    auto _duration = std::chrono::duration_cast<std::chrono::seconds>(_t2 - _t1).count();

    std::cout << _queue << std::endl << std::endl << std::flush;
    std::cout << "* " << MAX_THREADS_QUEUE << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;

    std::cout << std::endl << "total time: " << _duration << "s" << std::endl << std::flush;
    return 0;
}

auto
test_list() -> int {
    _pushed = 0;
    _poped = 0;
    auto _t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> _threads;

#ifdef INTERCEPT_SIGINT
    std::signal(SIGINT, pause);
#endif

    for (int _i = 0; _i != MAX_THREADS_LIST; ++_i) {
        _threads.emplace_back(
          [&](int _n_thread) {
              if (_n_thread % 2 == 0) {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS_LIST; ++_k) {
#ifdef QUEUE_USE_STRING
                          std::shared_ptr<std::string> _value = std::make_shared<std::string>(
                            std::to_string(_n_thread * N_ELEMENTS_LIST + _k));
#else
                          int _value = _n_thread * N_ELEMENTS_LIST + _k;
#endif
                          _list.push(_value);
                          ++_pushed;
                      }
                  }
                  catch (zpt::failed_expectation const& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
              else {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS_LIST;) {
                          try {
#ifdef QUEUE_USE_STRING
                              auto _it =
                                _list.erase([&](std::shared_ptr<std::string> const& _item) -> bool {
                                    return std::to_string((_n_thread - 1) * N_ELEMENTS_LIST + _k) ==
                                           *_item;
#else
                              auto _it = _list.erase([&](int const& _item) -> bool {
                                  return _item == (_n_thread - 1) * N_ELEMENTS_LIST + _k;
#endif
                                });
                              if (_it != _list.end()) {
                                  ++_poped;
                              }
                              ++_k;
                          }
                          catch (zpt::NoMoreElementsException const& e) {
                              std::this_thread::yield();
                          }
                      }
                  }
                  catch (zpt::failed_expectation const& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
          },
          _i);
    }

    for (int _i = 0; _i != MAX_THREADS_LIST; ++_i)
        _threads[_i].join();

    auto _t2 = std::chrono::high_resolution_clock::now();
    auto _duration = std::chrono::duration_cast<std::chrono::seconds>(_t2 - _t1).count();

    std::cout << _list << std::endl << std::endl << std::flush;
    std::cout << "* at index 0 is " << _list[0] << std::endl << std::endl << std::flush;

    std::cout << "* " << MAX_THREADS_LIST << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #removed -> " << _poped.load() << std::endl << std::flush;

    std::cout << std::endl << "total time: " << _duration << "s" << std::endl << std::flush;
    return 0;
}

auto
test_hazard_ptr() -> void {
    zpt::lf::queue<long>::hazard_domain _domain{ 2, 2 };
    zpt::lf::queue<long> _q1{ _domain };
    zpt::lf::queue<long> _q2{ _domain };

    _q1.push(1);
    _q1.pop();
    _q1.push(1);
    _q1.pop();
}

auto
test_aligned() -> void {
    zpt::padded_atomic<bool> _atomic{false};
    _atomic->store(true);
    std::cout << _atomic->load(std::memory_order_acquire) << std::endl << std::flush;
    (*_atomic) = false;
    std::cout << _atomic->load(std::memory_order_acquire) << std::endl << std::flush;
}

auto
main(int _argc, char* _argv[]) -> int {
    try {
        test_queue();
        // test_list();
        // test_hazard_ptr();
        // test_aligned();
        return 0;
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << _e.what() << std::endl
                  << _e.description() << std::endl
                  << _e.backtrace() << std::endl
                  << std::flush;
    }
    return 1;
}
