/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
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
