#pragma once

#include <memory>
#include <iterator>
#include <atomic>
#include <math.h>
#include <unistd.h>
#include <iostream>

namespace zpt {
namespace wf {
template <typename T> class fifo {
      public:
	using reference = T&;
	using pointer = T*;
	using size_type = size_t;
	class iterator;

	static const size_type first_bucket_size{8};
	static const size_type bucket_amount{50};

	fifo();
	fifo(const fifo<T>& copy);
	fifo(fifo<T>&& move);
	virtual ~fifo();

	fifo<T>& operator=(const fifo<T>& copy);
	fifo<T>& operator=(fifo<T>&& move);

	T at(size_type pos);
	T operator[](size_type pos);

	T front();
	T back();

	bool empty() const;
	size_type size() const;

	void clear();

	void push_back(const T& value);
	void push_back(T&& value);
	void pop_back();

	class iterator {
	      public:
		iterator(fifo* target);
		virtual ~iterator();

	      private:
		fifo* m_target;
	};

      protected:
	class write_op {
	      public:
		T m_old_value;
		T m_new_value;
		size_type m_position;

		write_op(T old_value, T new_value, size_type position);
	};

	class descriptor {
	      public:
		std::unique_ptr<write_op> m_pending;
		std::atomic_flag m_completed = ATOMIC_FLAG_INIT;

		descriptor(std::unique_ptr<write_op> op);
		descriptor(const descriptor& copy) = delete;
		descriptor(descriptor&& move) = delete;
		virtual ~descriptor();

		descriptor& operator=(const descriptor& copy) = delete;
		descriptor& operator=(descriptor&& move) = delete;
	};

      private:
	std::atomic<size_type> m_size;
	std::atomic<wf::fifo<T>::descriptor*> m_descriptor;
	std::atomic<std::atomic<T>*> m_memory[bucket_amount];

	void set(T value, size_type pos);
	void alloc_bucket(size_type bucket);
	void complete_write(descriptor* desc);
	std::pair<int, size_type> calculate(size_type idx);
};

template <typename T> zpt::wf::fifo<T>::fifo() : m_size(0), m_descriptor(nullptr) {
	this->m_memory[0] = new std::atomic<T>[fifo::first_bucket_size];
	for (size_t idx = 1; idx != bucket_amount; ++idx)
		this->m_memory[idx].store(nullptr);
}

template <typename T> zpt::wf::fifo<T>::~fifo() {
	for (size_t idx = 0; idx != bucket_amount; ++idx) {
		if (this->m_memory[idx] != nullptr) {
			delete[] this->m_memory[idx];
			this->m_memory[idx] = nullptr;
		}
	}
}

template <typename T> T zpt::wf::fifo<T>::at(size_type pos) {
	if (pos >= this->size())
		throw std::out_of_range("position out of bounds");

	std::pair<int, size_type> bucket_idx = this->calculate(pos);
	return this->m_memory[bucket_idx.first][bucket_idx.second];
}

template <typename T> T zpt::wf::fifo<T>::operator[](size_type pos) { return this->at(pos); }

template <typename T> T zpt::wf::fifo<T>::front() { return this->at(0); }

template <typename T> T zpt::wf::fifo<T>::back() { return this->at(this->size() - 1); }

template <typename T> bool zpt::wf::fifo<T>::empty() const { return true; }

template <typename T> size_t zpt::wf::fifo<T>::size() const { return this->m_size; }

template <typename T> void zpt::wf::fifo<T>::clear() {}

template <typename T> void zpt::wf::fifo<T>::push_back(const T& value) {
	size_type pos = this->m_size.fetch_add(1);
	this->set(value, pos);
}

template <typename T> void zpt::wf::fifo<T>::push_back(T&& value) {
	size_type pos = this->m_size.fetch_add(1);
	this->set(value, pos);
}

template <typename T> void zpt::wf::fifo<T>::pop_back() { this->m_size.fetch_sub(1); }

template <typename T> void zpt::wf::fifo<T>::set(T value, size_type pos) {
	zpt::wf::fifo<T>::descriptor* desc_current{nullptr};
	std::pair<int, size_type> bucket_idx = this->calculate(pos);

	if (this->m_memory[bucket_idx.first].load() == nullptr) {
		this->alloc_bucket(bucket_idx.first);
	}

	// std::cout << bucket_idx.first << ", " << bucket_idx.second << std::endl << std::flush;
	T old_value = this->m_memory[bucket_idx.first][bucket_idx.second];
	descriptor* desc_next = new descriptor{std::unique_ptr<write_op>{new write_op{old_value, value, pos}}};

	do {
		desc_current = this->m_descriptor.exchange(nullptr);
		if (desc_current != nullptr && !desc_current->m_completed.test_and_set()) {
			this->complete_write(desc_current);
			delete desc_current;
		}

		if (desc_next != nullptr && this->m_descriptor.compare_exchange_strong(desc_current, desc_next))
			desc_next = nullptr;

	} while (desc_next != nullptr || this->m_descriptor.load() != nullptr);
}

template <typename T> void zpt::wf::fifo<T>::alloc_bucket(size_type bucket) {
	size_type bucket_size = first_bucket_size * std::pow(2, bucket + 1);
	std::atomic<T>* new_bucket = new std::atomic<T>[bucket_size];
	std::atomic<T>* old_bucket = nullptr;
	if (!this->m_memory[bucket].compare_exchange_strong(old_bucket, new_bucket))
		delete[] new_bucket;
}

template <typename T> void zpt::wf::fifo<T>::complete_write(descriptor* desc) {
	if (desc->m_pending != nullptr) {
		std::pair<int, size_type> bucket_idx = this->calculate(desc->m_pending->m_position);
		this->m_memory[bucket_idx.first][bucket_idx.second].compare_exchange_strong(
		    desc->m_pending->m_old_value, desc->m_pending->m_new_value);
	}
}

template <typename T> std::pair<int, size_t> zpt::wf::fifo<T>::calculate(size_type idx) {
	size_type pos = idx + first_bucket_size;
	auto size_t_size = sizeof(idx);
	auto hibit = (size_t_size > 4 ? 63 - __builtin_clzll(pos) : 31 - __builtin_clz(pos));
	auto first_hibit =
	    (size_t_size > 4 ? 63 - __builtin_clzll(first_bucket_size) : 31 - __builtin_clz(first_bucket_size));
	auto to_xor = static_cast<uint64_t>(std::pow(2, hibit));
	auto bucket_idx = pos ^ to_xor;
	return std::make_pair(hibit - first_hibit, bucket_idx);
}

template <typename T>
zpt::wf::fifo<T>::descriptor::descriptor(std::unique_ptr<write_op> op) : m_pending(std::move(op)) {}

template <typename T> zpt::wf::fifo<T>::descriptor::~descriptor() {}

template <typename T>
zpt::wf::fifo<T>::write_op::write_op(T old_value, T new_value, size_type position)
    : m_old_value(old_value), m_new_value(new_value), m_position(position) {}

} // namespace wf
} // namespaace zpt
