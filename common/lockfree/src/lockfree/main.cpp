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

constexpr int N_ELEMENTS = 10000;
constexpr int MAX_THREADS = 1000;
constexpr int PER_THREAD = 8;

// #define QUEUE_USE_STRING
// #define INTERCEPT_SIGINT
// #define FIRST_PUSH_THEN_POP
#define SPIN_WAIT_MILLIS -2

std::atomic<int> _pushed{ 0 };
std::atomic<int> _poped{ 0 };

#ifdef QUEUE_USE_STRING
zpt::lf::queue<std::shared_ptr<std::string>> _queue{ MAX_THREADS, PER_THREAD, SPIN_WAIT_MILLIS };
zpt::lf::list<std::shared_ptr<std::string>> _list{ MAX_THREADS, PER_THREAD };
#else
zpt::lf::queue<int> _queue{ MAX_THREADS, PER_THREAD, SPIN_WAIT_MILLIS };
zpt::lf::list<int> _list{ MAX_THREADS, PER_THREAD };
#endif

auto
pause(int _signal) -> void {
    std::cout << _queue << std::endl << std::flush;
    std::cout << "* " << MAX_THREADS << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;
}

auto
main(int _argc, char* _argv[]) -> int {
    std::vector<std::thread> _threads;

#ifdef INTERCEPT_SIGINT
    std::signal(SIGINT, pause);
#endif

#ifdef FIRST_PUSH_THEN_POP
    for (int _i = 0; _i != MAX_THREADS / 2; ++_i) {
#else
    for (int _i = 0; _i != MAX_THREADS; ++_i) {
#endif
        _threads.emplace_back(
          [&](int _n_thread) {
#ifdef FIRST_PUSH_THEN_POP
              if (_n_thread < MAX_THREADS / 2) {
#else
              if (_n_thread % 2 == 0) {
#endif
                  try {
                      for (int _k = 0; _k != N_ELEMENTS; ++_k) {
#ifdef QUEUE_USE_STRING
                          std::shared_ptr<std::string> _value{ new std::string(
                            std::to_string(_n_thread * N_ELEMENTS + _k)) };
#else
                          int _value = _n_thread * N_ELEMENTS + _k + 1;
#endif
                          _queue.push(_value);
                          _list.push(_value);
                          ++_pushed;
                          std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 1 });
                      }
                  }
                  catch (zpt::ExpectationException& _e) {
                      std::cout << "ERROR: " << _e.what() << std::endl << std::flush;
                  }
              }
#ifndef FIRST_PUSH_THEN_POP
              else {
                  try {
                      for (int _k = 0; _k != N_ELEMENTS;) {
                          try {
#ifdef QUEUE_USE_STRING
                              std::shared_ptr<std::string> _value = _queue.pop();
#else
                              int _value = _queue.pop();
#endif
                              if (_poped % 2 == 0) {
                                  _list.erase(_value);
                              }
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
#endif
          },
          _i);
    }

#ifdef FIRST_PUSH_THEN_POP
    std::cout << "wainting for pushes to finish... " << std::flush;
    // std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 5000 });
    std::this_thread::yield();
    std::cout << "done!" << std::endl << std::flush;
    for (int _i = MAX_THREADS / 2; _i != MAX_THREADS; ++_i) {
        _threads.emplace_back(
          [](int _n_thread) {
              for (int _k = 0; _k != N_ELEMENTS;) {
                  try {
#ifdef QUEUE_USE_STRING
                      std::string* _value = _queue.pop();
                      _pops.push_back(*_value);
                      delete _value;
#else
                      int _value = _queue.pop();
#endif
                      ++_k;
                      ++_poped;
                  }
                  catch (zpt::NoMoreElementsException& e) {
                      // std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 10 });
                      std::this_thread::yield();
                  }
              }
          },
          _i);
    }
#endif

    for (int _i = 0; _i != MAX_THREADS; ++_i)
        _threads[_i].join();

    auto _it = _list.find(100);
    if (_it != _list.end()) {
        std::cout << "* found: " << (*_it) << std::endl << std::flush;
    }
    std::cout << _list << std::endl << std::flush;

    std::cout << _queue << std::endl << std::endl << std::flush;
    std::cout << "* " << MAX_THREADS << " working threads:" << std::endl << std::flush;
    std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;

    return 0;
}
