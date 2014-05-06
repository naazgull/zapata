#include <map/Mapper.h>

zapata::Mapper::Mapper(string _key_file_path) : zapata::Job(_key_file_path) {
}

zapata::Mapper::~Mapper() {
}

void zapata::Mapper::run() {
	for(; true; ) {
		this->wait();
		this->map();
		this->collect();
	}
}

