#pragma once

#include <zapata/core.h>

namespace zapata {

	template <typename I, typename O>
	class Mapper : public Job {
		public:
			Mapper();
			virtual ~Mapper();

			virtual void run();
			virtual void map(I* _in, O* _out) = 0;
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
void zapata::Mapper<I, O>::run() {
	for(; true; ) {
		this->wait();
		this->map(this->__in, this->__out);
		this->collect(this->__out);
	}
}

template <typename I, typename O>
I* zapata::Mapper<I, O>::in() {
	return this->__in;
}

template <typename I, typename O>
O* zapata::Mapper<I, O>::out() {
	return this->__in;
}


template <typename I, typename O>
void zapata::Mapper<I, O>::in(I* _in) {
	this->__in = _in;
}

template <typename I, typename O>
void zapata::Mapper<I, O>::out(O*_out) {
	this->__out = _out;
}

