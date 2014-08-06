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

#pragma once

#include <string.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <text/convert.h>
#include <mem/usage.h>

#define HASH_SIZE 50

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	unsigned int djb2(string key);

	enum ObjectOp {
		pretty = 1, minified = 2, json = 4,  xml = 8,  undefined = 16,  headers = 32,  params = 64,  body = 128
	};

	template<typename K, typename V>
	class duo {
		public:
			duo(K k, V value);
			virtual ~duo();

		public:
			K first;
			V second;
			size_t third;

			friend ostream& operator<<(ostream& _out, duo& _in) {
				_out << "duo<" << _in.first << ", " << *_in.second << ", " << _in.third << ">";
				return _out;
			}
	};

	template<typename V>
	bool _compare(duo<string, V>* lh, duo<string, V>* rh);

	template<typename V>
	class str_map {
		public:
			str_map(size_t hash_size = HASH_SIZE);
			virtual ~str_map();

			typedef typename vector<vector<duo<string, V>*>*>::iterator hashtable_iterator;
			typedef typename vector<duo<string, V>*>::iterator bucket_iterator;
			typedef bucket_iterator hash_iterator;

			class iterator: public std::iterator<std::random_access_iterator_tag, duo<string, V>*> {

				public:
					inline iterator() {
					}
					inline iterator(const iterator& other) :
						_underlying(other._underlying), _idx(other._idx) {
					}
					inline iterator(iterator& other) :
						_underlying(other._underlying), _idx(other._idx) {
					}
					inline iterator(vector<duo<string, V>*>* other, size_t pos = 0) :
						_underlying(other), _idx(pos) {
					}
					inline ~iterator() {
					}

					inline iterator& operator++() {
						++this->_idx;
						return *this;
					}

					inline iterator operator++(int n) {
						iterator tmp(*this);
						operator++();
						return tmp;
					}

					inline iterator& operator--() {
						--this->_idx;
						return *this;
					}

					inline iterator operator--(int n) {
						iterator tmp(*this);
						operator--();
						return tmp;
					}

					inline iterator& operator+(size_t n) {
						this->_idx += n;
						return *this;
					}

					inline iterator& operator-(size_t n) {
						this->_idx -= n;
						return *this;
					}

					inline bool operator==(const iterator& rhs) {
						return this->_underlying == rhs._underlying && this->_idx == rhs._idx;
					}

					inline bool operator!=(const iterator& rhs) {
						return this->_underlying != rhs._underlying || this->_idx != rhs._idx;
					}

					inline duo<string, V>* operator*() {
						return *(this->_underlying->begin() + this->_idx);
					}

					inline duo<string, V>* operator->() {
						return *(this->_underlying->begin() + this->_idx);
					}

				private:
					vector<duo<string, V>*>* _underlying;
					size_t _idx;

			};

			class ordered_iterator: public std::iterator<std::random_access_iterator_tag, V> {

				public:
					inline ordered_iterator() {
					}
					inline ordered_iterator(const ordered_iterator& other) :
						_underlying(other._underlying), _idx(other._idx) {
					}
					inline ordered_iterator(ordered_iterator& other) :
						_underlying(other._underlying), _idx(other._idx) {
					}
					inline ordered_iterator(vector<duo<string, V>*>* other, size_t pos = 0) :
						_underlying(other), _idx(pos) {
					}
					inline ~ordered_iterator() {
					}

					inline ordered_iterator& operator++() {
						++this->_idx;
						return *this;
					}

					inline ordered_iterator operator++(int n) {
						ordered_iterator tmp(*this);
						operator++();
						return tmp;
					}

					inline ordered_iterator& operator--() {
						--this->_idx;
						return *this;
					}

					inline ordered_iterator operator--(int n) {
						ordered_iterator tmp(*this);
						operator--();
						return tmp;
					}

					inline ordered_iterator& operator+(size_t n) {
						this->_idx += n;
						return *this;
					}

					inline ordered_iterator& operator-(size_t n) {
						this->_idx -= n;
						return *this;
					}

					inline bool operator==(const ordered_iterator& rhs) {
						return this->_underlying == rhs._underlying && this->_idx == rhs._idx;
					}

					inline bool operator!=(const ordered_iterator& rhs) {
						return this->_underlying != rhs._underlying || this->_idx != rhs._idx;
					}

					inline V& operator*() {
						duo<string, V>* d = *(this->_underlying->begin() + this->_idx);
						return d->second;
					}

					inline V* operator->() {
						return &((*(this->_underlying->begin() + this->_idx))->second);
					}

				private:
					vector<duo<string, V>*>* _underlying;
					size_t _idx;

			};

			void set_deleted_key(string k);
			void clear();
			void resize(size_t k);

			iterator begin();
			iterator end();

			iterator begin_by_key();
			iterator end_by_key();

			ordered_iterator begin_by_value();
			ordered_iterator end_by_value();

			V& at(size_t idx);
			iterator find(string key);
			void erase(iterator);
			void insert(pair<string, V> v);
			void insert(pair<size_t, V> v);
			void insert(string k, V v);
			void insert(size_t k, V v);

			void sort();
			void move(string name, int offset);

			size_t size();

			V& operator[](int idx);
			V& operator[](const char* name);
			V& operator[](string& name);

			inline friend ostream& operator<<(ostream& os, iterator& f) {
				os << "iterator pointing to " << f->second;
				return os;
			}

			inline friend ostream& operator<<(ostream& os, ordered_iterator& f) {
				os << "iterator pointing to " << *f;
				return os;
			}

			inline friend ostream& operator<<(ostream& os, str_map<V>& f) {
				f.print(os);
				return os;
			}

			void print(ostream& os = cout);

		private:
			vector<duo<string, V>*>** __hash_table;
			vector<duo<string, V>*>* __hash_values;
			size_t __hash_size;

			void _rellocate(duo<string, V>* v, size_t offset);
			duo<string, V>* _at(size_t idx);
			hash_iterator _find(string key);
	};

}

template<typename K, typename V>
zapata::duo<K, V>::duo(K kp, V vp) :
	first(kp), second(vp), third(-1) {
}

template<typename K, typename V>
zapata::duo<K, V>::~duo() {
	if (zapata::is_pointer<V>::value) {
		delete this->second;
	}
}

template<typename V>
zapata::str_map<V>::str_map(size_t hash_size) :
	__hash_table(new vector<duo<string, V>*>*[hash_size]), __hash_values(new vector<duo<string, V>*>()), __hash_size(hash_size) {
	bzero(this->__hash_table, this->__hash_size * sizeof(vector<duo<string, V>*>*));
}

template<typename V>
zapata::str_map<V>::~str_map() {
	for (size_t i = 0; i != this->__hash_size; i++) {
		if (this->__hash_table[i] == NULL) {
			continue;
		}
		delete this->__hash_table[i];
	}
	for (zapata::str_map<V>::bucket_iterator k = this->__hash_values->begin(); k != this->__hash_values->end(); k++) {
		delete *k;
	}
	delete[] this->__hash_table;
	delete this->__hash_values;
}

template<typename V>
void zapata::str_map<V>::set_deleted_key(string k) {
}

template<typename V>
void zapata::str_map<V>::clear() {
}

template<typename V>
void zapata::str_map<V>::resize(size_t k) {
}

template<typename V>
typename zapata::str_map<V>::iterator zapata::str_map<V>::begin() {
	return iterator(this->__hash_values);
}

template<typename V>
typename zapata::str_map<V>::iterator zapata::str_map<V>::end() {
	return iterator(this->__hash_values, this->__hash_values->size());
}

template<typename V>
typename zapata::str_map<V>::iterator zapata::str_map<V>::begin_by_key() {
	return iterator(this->__hash_values);
}

template<typename V>
typename zapata::str_map<V>::iterator zapata::str_map<V>::end_by_key() {
	return iterator(this->__hash_values, this->__hash_values->size());
}

template<typename V>
typename zapata::str_map<V>::ordered_iterator zapata::str_map<V>::begin_by_value() {
	return ordered_iterator(this->__hash_values);
}

template<typename V>
typename zapata::str_map<V>::ordered_iterator zapata::str_map<V>::end_by_value() {
	return ordered_iterator(this->__hash_values, this->__hash_values->size());
}

template<typename V>
void zapata::str_map<V>::insert(pair<string, V> v) {
	this->insert(v.first, v.second);
}

template<typename V>
void zapata::str_map<V>::insert(pair<size_t, V> v) {
	this->insert(v.first, v.second);
}

template<typename V>
void zapata::str_map<V>::insert(string k, V v) {
	unsigned int hash = zapata::djb2(k);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	if (bucket != NULL) {
		for (zapata::str_map<V>::bucket_iterator kt = bucket->begin(); kt != bucket->end(); kt++) {
			if ((*kt)->first == k) {
				if (zapata::is_pointer<V>::value) {
					delete (*kt)->second;
				}
				(*kt)->second = v;
				return;
			}
		}
	}
	else {
		bucket = new vector<duo<string, V>*>();
		this->__hash_table[hash] = bucket;
	}

	duo < string, V > *d = new duo<string, V>(k, v);
	bucket->push_back(d);
	d->third = this->__hash_values->size();
	this->__hash_values->push_back(d);
}

template<typename V>
void zapata::str_map<V>::insert(size_t k, V v) {
	string oss;
	zapata::tostr(oss, k);
	duo < string, V > *d = new duo<string, V>(oss, v);
	unsigned int hash = zapata::djb2(d->first);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	if (bucket == NULL) {
		bucket = new vector<duo<string, V>*>();
		this->__hash_table[hash] = bucket;
	}
	bucket->push_back(d);
	d->third = k;

	size_t init_pos = this->size() - 1;
	for (size_t it = init_pos; it >= k; --it) {
		duo < string, V > *val = this->__hash_values->at(it);
		this->_rellocate(val, 1);
	}

	this->__hash_values->insert(this->__hash_values->begin() + k, d);
}

template<typename V>
V& zapata::str_map<V>::at(size_t idx) {
	return this->__hash_values->at(idx)->second;
}

template<typename V>
typename zapata::str_map<V>::iterator zapata::str_map<V>::find(string key) {
	unsigned int hash = zapata::djb2(key);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	if (bucket == NULL) {
		return this->end();
	}

	for (zapata::str_map<V>::bucket_iterator k = bucket->begin(); k != bucket->end(); k++) {
		if ((*k)->first == key) {
			return this->begin() + (*k)->third;
		}
	}

	return this->end();
}

template<typename V>
void zapata::str_map<V>::erase(zapata::str_map<V>::iterator i) {
	unsigned int hash = zapata::djb2(i->first);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	if (bucket == NULL) {
		delete *i;
		return;
	}

	for (zapata::str_map<V>::bucket_iterator k = bucket->begin(); k != bucket->end(); k++) {
		if ((*k)->first == i->first) {
			bucket->erase(k);
			break;
		}
	}

	size_t pos = i->third;
	delete *i;
	this->__hash_values->erase(this->__hash_values->begin() + pos);
	for (size_t k = 0; k != this->__hash_values->size(); k++) {
		this->__hash_values->at(k)->third = k;
	}
}

template<typename V>
size_t zapata::str_map<V>::size() {
	return this->__hash_values->size();
}

template<typename V>
void zapata::str_map<V>::sort() {
	std::sort(this->__hash_values->begin(), this->__hash_values->end(), zapata::_compare<V>);
	for (size_t k = 0; k != this->__hash_values->size(); k++) {
		this->__hash_values->at(k)->third = k;
	}

}

template<typename V>
void zapata::str_map<V>::move(string name, int offset) {
	unsigned int hash = zapata::djb2(name);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	if (bucket == NULL) {
		return;
	}

	duo < string, V > *val = NULL;
	for (zapata::str_map<V>::bucket_iterator k = bucket->begin(); k != bucket->end(); k++) {
		if ((*k)->first == name) {
			val = *k;
			break;
		}
	}
	if (val == NULL) {
		return;
	}

	size_t step = 0;
	if (offset < 0) {
		step = -1;
	}
	else {
		step = 1;
	}
	for (size_t it = val->third + step; it != val->third + offset; it = it + step) {
		duo < string, V > *other = this->__hash_values->at(it);
		this->_rellocate(other, step);
	}

	this->__hash_values->erase(this->__hash_values->begin() + val->third);
	val->third += offset;
	this->__hash_values->insert(this->__hash_values->begin() + val->third, val);
}

template<typename V>
V& zapata::str_map<V>::operator[](int idx) {
	return this->at((size_t) idx);
}

template<typename V>
V& zapata::str_map<V>::operator[](const char* name) {
	V& ret =  this->find(string(name))->second;
	return ret;
}

template<typename V>
V& zapata::str_map<V>::operator[](string& name) {
	return this->find(name)->second;
}

template<typename V>
void zapata::str_map<V>::print(ostream& out) {
	out << endl << "str_map<" << ((void*) this) << "> _underlying<" << this->__hash_values << ":" << this->__hash_values->size() << ">" << endl << flush;
	for (size_t k = 0; k != this->__hash_values->size(); k++) {
		out << "\t[" << k << "] " << this->__hash_values->at(k)->first << ": " << *this->__hash_values->at(k)->second << endl << flush;
	}
}

template<typename V>
zapata::duo<string, V>* zapata::str_map<V>::_at(size_t idx) {
	return this->__hash_values->at(idx);
}

template<typename V>
typename zapata::str_map<V>::hash_iterator zapata::str_map<V>::_find(string key) {
	unsigned int hash = zapata::djb2(key);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	if (bucket == NULL) {
		return this->__hash_values->end();
	}

	for (zapata::str_map<V>::bucket_iterator k = bucket->begin(); k != bucket->end(); k++) {
		if ((*k)->first == key) {
			return this->__hash_values->begin() + (*k)->third;
		}
	}

	return this->__hash_values->end();
}

template<typename V>
bool zapata::_compare(duo<string, V>* lh, duo<string, V>* rh) {
	return lh->first.compare(rh->first) <= 0;
}

template<typename V>
void zapata::str_map<V>::_rellocate(duo<string, V>* val, size_t offset) {
	unsigned int hash = zapata::djb2(val->first);

	vector<duo<string, V>*>* bucket = this->__hash_table[hash];
	for (zapata::str_map<V>::bucket_iterator kt = bucket->begin(); kt != bucket->end(); kt++) {
		if ((*kt)->first == val->first) {
			bucket->erase(kt);
			break;
		}
	}

	val->third += offset;
	zapata::tostr(val->first, val->third);
	hash = zapata::djb2(val->first);

	bucket = this->__hash_table[hash];
	if (bucket == NULL) {
		bucket = new vector<duo<string, V>*>();
		this->__hash_table[hash] = bucket;
	}
	bucket->push_back(val);
}

