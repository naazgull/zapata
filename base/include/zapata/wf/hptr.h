#pragma once

#include <memory>
#include <iterator>
#include <atomic>
#include <math.h>
#include <unistd.h>
#include <iostream>

namespace zpt {
namespace wf {

template <typename T> class hptr {
      public:
	using reference = T&;
	using pointer = T*;
	using size_type = size_t;

	hptr() = default;
	hptr(const hptr<T>& copy);
	hptr(hptr<T>& move);
	virtual ~hptr() = default;

	hptr<T>& operator=(const hptr<T>& copy);
	hptr<T>& operator=(hptr<T>&& move);

      private:
	pointer m_target;
	zpt::wf::hptr<T>* m_next;
};

template <typename T> class hptr_domain {
      public:
	using reference = T&;
	using pointer = T*;
	using size_type = size_t;

	static const int CLPAD = 128 / sizeof(std::atomic<T*>);

	hptr_domain(int n_threads, int n_hp_per_thread);
	hptr_domain(const hptr_domain<T>& copy);
	hptr_domain(hptr_domain<T>& move);
	virtual ~hptr_domain();

	hptr_domain<T>& operator=(const hptr_domain<T>& copy);
	hptr_domain<T>& operator=(hptr_domain<T>&& move);

      private:
	int P;
	int K;
	int N;
	int R;
	std::atomic<pointer>* m_hp;
	thread_local static pointer m_retired;
};

template <typename T>
zpt::wf::hptr_domain<T>::hptr_domain(int n_threads, int n_hp_per_thread)
    : P(n_threads), K(n_hp_per_thread), N(P * K), R(2 * N) {
	this->m_hp = new std::atomic<pointer>[N];
}

template <typename T> zpt::wf::hptr_domain<T>::~hptr_domain() { delete this->m_hp; }

} // namespace wf
} // namespaace zpt
