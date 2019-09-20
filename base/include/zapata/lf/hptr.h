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
#pragma once

#include <atomic>
#include <iostream>
#include <iterator>
#include <map>
#include <math.h>
#include <memory>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <zapata/base/assertz.h>

namespace zpt {
namespace lf {

template<typename T, int MAX_T, int PER_T>
class hptr_domain {
  public:
    using size_type = size_t;
    using hptr_pending_list = std::map<T*, T*>;

    static constexpr int CACHE_LINE_PADDING{ 128 / sizeof(std::atomic<T*>) };
    static constexpr int FACTOR{ static_cast<int>(
      std::ceil(static_cast<double>(PER_T) / static_cast<double>(CACHE_LINE_PADDING))) };
    static constexpr int P{ MAX_T };
    static constexpr int K{ FACTOR * CACHE_LINE_PADDING };
    static constexpr long N{ P * K };
    static constexpr long R{ N };

    hptr_domain(const hptr_domain<T, MAX_T, PER_T>& _rhs) = delete;
    hptr_domain(hptr_domain<T, MAX_T, PER_T>&& _rhs) = delete;
    virtual ~hptr_domain();

    auto operator=(const hptr_domain<T, MAX_T, PER_T>& _rhs)
      -> hptr_domain<T, MAX_T, PER_T>& = delete;
    auto operator=(hptr_domain<T, MAX_T, PER_T>&& _rhs) -> hptr_domain<T, MAX_T, PER_T>& = delete;

    auto acquire(T* _ptr) -> hptr_domain<T, MAX_T, PER_T>&;
    auto release(T* _ptr) -> hptr_domain<T, MAX_T, PER_T>&;
    auto retire(T* _ptr) -> hptr_domain<T, MAX_T, PER_T>&;
    auto clean() -> hptr_domain<T, MAX_T, PER_T>&;

    static auto get_instance() -> hptr_domain<T, MAX_T, PER_T>&;
    static auto get_this_thread_idx() -> int;
    static auto clean_this_thread() -> void;

    friend auto operator<<(std::ostream& _out, zpt::lf::hptr_domain<T, MAX_T, PER_T>& _in)
      -> std::ostream& {
        std::cout << "#hazards:\n\talive -> " << std::dec << _in.__alive.load() << std::endl
                  << "\tretired -> " << _in.__retired.load() << std::endl
                  << std::flush;
        _out << std::dec << std::flush;
        return _out;
    }

    class guard {
      public:
        guard(T* _target);
        virtual ~guard();

        auto retire() -> guard&;
        auto target() const -> T&;

      private:
        T* __target{ nullptr };
        bool __retire{ false };
    };

  private:
    std::atomic<T*> __hp[N];
    std::atomic<int> __next_thr_idx{ 0 };
    std::atomic<long> __alive{ 0 };
    std::atomic<long> __retired{ 0 };

    hptr_domain() = default;

    static auto get_retired() -> hptr_pending_list&;
};

template<typename T, int MAX_T, int PER_T>
zpt::lf::hptr_domain<T, MAX_T, PER_T>::~hptr_domain() {
    auto& _retired = zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_retired();
    for (auto [_ptr, _ignore] : _retired) {
        delete _ptr;
    }
    _retired.clear();
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::acquire(T* _ptr) -> zpt::lf::hptr_domain<T, MAX_T, PER_T>& {
    size_t _idx = zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_this_thread_idx();

    for (size_t _k = _idx * K; _k != static_cast<size_t>((_idx + 1) * K); ++_k) {
        T* _null{ nullptr };
        if (this->__hp[_k].compare_exchange_strong(_null, _ptr)) {
            this->__alive++;
            return (*this);
        }
    }

    assertz(false,
            "No more hazard-pointer slots available for this thread. "
            "Please, run the `clean()` "
            "before continuing",
            500,
            0);
    return (*this);
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::release(T* _ptr) -> zpt::lf::hptr_domain<T, MAX_T, PER_T>& {
    int _idx{ zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_this_thread_idx() };

    for (size_t _k = _idx * K; _k != static_cast<size_t>((_idx + 1) * K); ++_k) {
        T* _exchange = _ptr;
        if (this->__hp[_k].compare_exchange_strong(_exchange, nullptr)) {
            this->__alive--;
            break;
        }
    }
    return (*this);
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::retire(T* _ptr) -> zpt::lf::hptr_domain<T, MAX_T, PER_T>& {
    auto& _retired = zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_retired();
    _retired.insert(std::make_pair(_ptr, _ptr));
    if (_retired.size() == static_cast<size_t>(R)) {
        this->clean();
    }
    return (*this);
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::clean() -> zpt::lf::hptr_domain<T, MAX_T, PER_T>& {
    auto& _retired = zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_retired();

    std::map<T*, size_t> _to_process;
    for (size_t _idx = 0; _idx != N; ++_idx) {
        T* _ptr = this->__hp[_idx].load();
        if (_ptr != nullptr)
            _to_process.insert(std::make_pair(_ptr, _idx));
    }

    for (auto _it = _retired.begin(); _it != _retired.end(); ++_it) {
        if (_to_process.find(_it->first) == _to_process.end()) {
            delete _it->first;
            this->__retired++;
            _it = _retired.erase(_it);
        }
    }

    return (*this);
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_instance() -> zpt::lf::hptr_domain<T, MAX_T, PER_T>& {
    static zpt::lf::hptr_domain<T, MAX_T, PER_T> _instance;
    return _instance;
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_retired() -> std::map<T*, T*>& {
    thread_local static hptr_pending_list _retired;
    return _retired;
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_this_thread_idx() -> int {
    thread_local static int _idx =
      zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_instance().__next_thr_idx.fetch_add(1);
    return _idx;
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::clean_this_thread() -> void {
    auto& _retired = zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_retired();
    for (auto _hptr : _retired) {
        delete _hptr.first;
    }
    _retired.clear();
}

template<typename T, int MAX_T, int PER_T>
zpt::lf::hptr_domain<T, MAX_T, PER_T>::guard::guard(T* _ptr)
  : __target{ _ptr } {
    zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_instance().acquire(this->__target);
}

template<typename T, int MAX_T, int PER_T>
zpt::lf::hptr_domain<T, MAX_T, PER_T>::guard::~guard() {
    zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_instance().release(this->__target);
    if (this->__retire)
        zpt::lf::hptr_domain<T, MAX_T, PER_T>::get_instance().retire(this->__target);
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::guard::retire() -> guard& {
    this->__retire = true;
    return (*this);
}

template<typename T, int MAX_T, int PER_T>
auto
zpt::lf::hptr_domain<T, MAX_T, PER_T>::guard::target() const -> T& {
    return *this->__target;
}

} // namespace lf
} // namespace zpt
