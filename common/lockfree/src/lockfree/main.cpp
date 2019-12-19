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
#include <zapata/lockfree/queue.h>
#include <zapata/lockfree/list.h>
#include <zapata/log/log.h>
#include <zapata/text/manip.h>
#include <zapata/exceptions/NoMoreElementsException.h>

constexpr int N_ELEMENTS_QUEUE = 10000;
constexpr int MAX_THREADS_QUEUE = 1000;

constexpr int N_ELEMENTS_LIST = 1000;
constexpr int MAX_THREADS_LIST = 50;

constexpr int PER_THREAD = 8;

// #define QUEUE_USE_STRING
// #define INTERCEPT_SIGINT
#define SPIN_WAIT_MILLIS -2

std::atomic<int> _pushed{ 0 };
std::atomic<int> _poped{ 0 };

#ifdef QUEUE_USE_STRING
zpt::lf::queue<std::shared_ptr<std::string>> _queue{ MAX_THREADS_QUEUE,
                                                     PER_THREAD,
                                                     SPIN_WAIT_MILLIS };
zpt::lf::list<std::shared_ptr<std::string>> _list{ MAX_THREADS_LIST, PER_THREAD };
#else
zpt::lf::queue<int> _queue{ MAX_THREADS_QUEUE, PER_THREAD, SPIN_WAIT_MILLIS };
zpt::lf::list<int> _list{ MAX_THREADS_LIST, PER_THREAD };
#endif

auto
pause(int _signal) -> void {
    std::cout << _queue << std::endl << std::flush;
    std::cout << "* " << MAX_THREADS_QUEUE << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;
}

auto
test_queue(int _argc, char* _argv[]) -> int {
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
                  catch (zpt::ExpectationException& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
              else {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS_QUEUE;) {
                          try {
#ifdef QUEUE_USE_STRING
                              std::shared_ptr<std::string> _value = _queue.pop();
#else
                              int _value = _queue.pop();
#endif
                              ++_k;
                              ++_poped;
                          }
                          catch (zpt::NoMoreElementsException& e) {
                              std::this_thread::yield();
                          }
                      }
                  }
                  catch (zpt::ExpectationException& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
          },
          _i);
    }

    for (int _i = 0; _i != MAX_THREADS_QUEUE; ++_i)
        _threads[_i].join();

    std::cout << _queue << std::endl << std::endl << std::flush;
    std::cout << "* " << MAX_THREADS_QUEUE << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;

    return 0;
}

auto
test_list(int _argc, char* _argv[]) -> int {
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
                          std::shared_ptr<std::string> _value{ new std::string(
                            std::to_string(_n_thread * N_ELEMENTS_LIST + _k)) };
#else
                          int _value = _n_thread * N_ELEMENTS_LIST + _k;
#endif
                          _list.push(_value);
                          ++_pushed;
                      }
                  }
                  catch (zpt::failed_expectation& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
              else {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS_LIST;) {
                          try {
#ifdef QUEUE_USE_STRING
                              _list.erase([&](std::shared_ptr<std::string> const& _item) -> bool {
                                  return std::to_string((_n_thread - 1) * N_ELEMENTS_LIST + _k) ==
                                         *_item;
#else
                              _list.erase([&](int const& _item) -> bool {
                                  return _item == (_n_thread - 1) * N_ELEMENTS_LIST + _k;
#endif
                              });
                              ++_poped;
                              ++_k;
                          }
                          catch (zpt::NoMoreElementsException& e) {
                              std::this_thread::yield();
                          }
                      }
                  }
                  catch (zpt::failed_expectation& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
          },
          _i);
    }

    for (int _i = 0; _i != MAX_THREADS_LIST; ++_i)
        _threads[_i].join();

    std::cout << _list << std::endl << std::endl << std::flush;
#ifdef QUEUE_USE_STRING
    auto _it = _list.find([&](std::shared_ptr<std::string> const& _item) -> bool {
        return *_item == "5";
#else
    auto _it = _list.find([&](int const& _item) -> bool {
        return _item == 5;
#endif
    });
    if (_it != _list.end()) {
        std::cout << "* found: " << (*_it) << std::endl << std::flush;
    }

    std::cout << "* at index 100 is " << _list[100] << std::endl << std::flush;

    return 0;
}

auto
main(int _argc, char* _argv[]) -> int {
    // return test_queue(_argc, _argv);
    return test_list(_argc, _argv);
}
