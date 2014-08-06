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

#include <base/str_map.h>

unsigned int zapata::djb2(string key) {
    unsigned int hash = 5381;

    for (unsigned int i = 0; i < key.length(); i++)
        hash = ((hash << 5) + hash) + (unsigned int) key[i];

    return hash % HASH_SIZE;
}
