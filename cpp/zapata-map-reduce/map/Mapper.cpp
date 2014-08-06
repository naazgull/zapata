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

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

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

