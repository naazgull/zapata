/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <base/smart_ptr.h>

namespace zapata {
	pthread_key_t __memory_key;
}

zapata::smart_counter::smart_counter() : __pointed(0) {
}

zapata::smart_counter::~smart_counter() {
}

void zapata::smart_counter::add() {
	this->__pointed++;
}

size_t zapata::smart_counter::release() {
	if (this->__pointed != 0) {
		this->__pointed--;
	}
	return this->__pointed;
}

zapata::smart_ref_table::smart_ref_table() : str_map(HASH_SIZE) {
}

zapata::smart_ref_table::~smart_ref_table() {
}

zapata::smart_counter* zapata::smart_ref_table::add(void* ptr) {
	ostringstream oss;
	oss << ptr << flush;
	str_map<smart_counter*>::iterator found = this->find(oss.str());
	if (found != this->end()) {
		(*found)->second->add();
		return (*found)->second;
	}
	else {
		smart_counter* sc = new smart_counter();
		this->insert(pair<string, smart_counter*>(oss.str(), sc));
		sc->add();
		return sc;
	}
}

size_t zapata::smart_ref_table::release(void* ptr) {
	ostringstream oss;
	oss << ptr << flush;
	str_map<smart_counter*>::iterator found = this->find(oss.str());
	if (found != this->end()) {
		return (*found)->second->release();
	}

	return 0;
}

void zapata::smart_ref_table::remove(void* ptr) {
	ostringstream oss;
	oss << ptr << flush;
	str_map<smart_counter*>::iterator found = this->find(oss.str());
	if (found != this->end()) {
		this->erase(found);
	}
}
