#include <reduce/Reducer.h>

zapata::Reducer::Reducer(string _key_file_path) : zapata::Job(_key_file_path) {
}

zapata::Reducer::~Reducer() {
}

void zapata::Reducer::run() {
	for(; true; ) {
		this->wait();
		this->reduce();
		this->collect();
	}
}
