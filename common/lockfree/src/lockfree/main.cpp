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

// #define WITH_ATOMIC_SHARED_PTR

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
#include <zapata/atomics/padded_atomic.h>
#include <zapata/lockfree/queue.h>
#include <zapata/log/log.h>
#include <zapata/text/manip.h>
#include <zapata/exceptions/NoMoreElementsException.h>

constexpr int N_ELEMENTS_QUEUE = 1;
constexpr int MAX_THREADS_QUEUE = 2;

constexpr int PER_THREAD = 2;

// #define QUEUE_USE_STRING
// #define INTERCEPT_SIGINT
#define SPIN_WAIT_MICROS 5

std::atomic<int> _pushed{ 0 };
std::atomic<int> _poped{ 0 };

#ifdef QUEUE_USE_STRING
using item_type = std::shared_ptr<std::string>;
#else
using item_type = int;
#endif
zpt::lf::queue<item_type> _queue{ MAX_THREADS_QUEUE };

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
                          _queue.push(std::move(_value));
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
              _queue.clear_thread_context();
          },
          _i);
    }

    for (int _i = 0; _i != MAX_THREADS_QUEUE; ++_i) _threads[_i].join();
    // auto _t2 = std::chrono::high_resolution_clock::now();
    // auto _duration = std::chrono::duration_cast<std::chrono::milliseconds>(_t2 - _t1).count();

    // std::cout << _queue << std::endl << std::endl << std::flush;
    // std::cout << "* " << MAX_THREADS_QUEUE << " working threads:" << std::endl << std::flush;
    // std::cout << "  #pushed -> " << _pushed.load() << std::endl << std::flush;
    // std::cout << "  #poped -> " << _poped.load() << std::endl << std::flush;

    // std::cout << std::endl << "total time: " << _duration << "ms" << std::endl << std::flush;
    return 0;
}

auto
test_hazard_ptr() -> void {
    zpt::lf::queue<long> _q1{ 2 };
    zpt::lf::queue<long> _q2{ 2 };

    _q1.push(1);
    _q1.push(2);
    _q1.push(3);
    _q1.push(4);
    _q1.push(5);
    _q1.push(6);
    _q1.pop();
    _q1.push(7);
    _q1.pop();
    _q1.push(8);
    _q1.push(9);
    _q1.push(10);
    _q1.push(11);
    _q1.push(12);
    _q1.push(13);

    std::cout << _q1 << std::endl << "front: " << _q1.front() << std::endl << std::flush;
}

auto
test_aligned() -> void {
    zpt::padded_atomic<bool> _atomic{ false };
    _atomic->store(true);
    std::cout << alignof(std::atomic<bool>) << " " << alignof(zpt::padded_atomic<bool>)
              << " with value of " << std::boolalpha << _atomic->load(std::memory_order_acquire)
              << std::endl
              << std::flush;
    (*_atomic) = false;
    std::cout << _atomic->load(std::memory_order_acquire) << std::endl << std::flush;
}

auto
main(int _argc, char* _argv[]) -> int {
    test_queue();
    // test_hazard_ptr();
    // test_aligned();
    return 0;
}
