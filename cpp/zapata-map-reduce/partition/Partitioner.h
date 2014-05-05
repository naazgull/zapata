#pragma once

#include <zapata/core.h>

namespace zapata {

	template <typename I, typename O>
	class Partitioner : public Job {
		public:
			Partitioner();
			virtual ~Partitioner();

			virtual void run();
			virtual void divide(I _in, O _out) = 0;
			virtual void collect(O* _out) = 0;

			virtual I* in();
			virtual O* out();

			virtual void in(I* _in);
			virtual void out(O* _out);

		private:
			I* __in;
			O* __out;
	};
}

template <typename I, typename O>
void zapata::Partitioner<I, O>::run() {
	for(; true; ) {
		this->wait();
		this->divide(this->__in, this->__out);
		this->collect(this->__out);
	}
}

template <typename I, typename O>
I* zapata::Partitioner<I, O>::in() {
	return this->__in;
}

template <typename I, typename O>
O* zapata::Partitioner<I, O>::out() {
	return this->__in;
}


template <typename I, typename O>
void zapata::Partitioner<I, O>::in(I* _in) {
	this->__in = _in;
}

template <typename I, typename O>
void zapata::Partitioner<I, O>::out(O*_out) {
	this->__out = _out;
}

