#include <partition/Partitioner.h>

zapata::Partitioner::Partitioner(string _key_file_path) : zapata::Job(_key_file_path) {
}

zapata::Partitioner::~Partitioner() {
}

void zapata::Partitioner::run() {
	for(; true; ) {
		this->wait();
		this->divide();
		this->collect();
	}
}

