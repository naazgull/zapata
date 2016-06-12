/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

#pragma once

#include <string>
#include <zapata/json/JSONObj.h>
#include <zapata/json/JSONParser.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class csv : public std::string {
	public: 
		inline csv() : std::string() {
		};
		inline csv(std::string _rhs) : std::string(_rhs) {
		};
		inline csv(const char * _rhs) : std::string(_rhs) {
		};
		friend ostream& operator<<(ostream& _out, zpt::csv& _in) {
			_out << string(_in.data());
			return _out;
		};
		friend istream& operator>>(istream& _in, zpt::csv& _out) {
			_out.clear();
			std::getline(_in, _out, '\n');
			zpt::trim(_out);
			return _in;
		};
		inline operator zpt::JSONPtr() {
			zpt::JSONPtr _result = zpt::mkarr();
			std::istringstream _iss;
			_iss.str(* this);

			char _c = '\0';
			bool _commas = false;
			bool _escaped = false;
			std::string _value;
			do {
				_iss >> _c;
				switch (_c) {
					case '\0' : {
						break;
					}
					case '"' : {
						if (_escaped) {
							_escaped = false;
							_value.push_back(_c);
						}
						else {
							_commas = !_commas;
						}
						break;
					}
					case '\\' : {
						_escaped = true;
						break;
					}
					case ',' : {
						if (!_commas) {
							_result << _value;
							_value.clear();
						}
						else {
							_value.push_back(_c);
						}
						break;
					}
					default : {
						_value.push_back(_c);
						break;
					}
				}
				_c = '\0';
			}
			while(_iss.good());
			_result << _value;

			return _result;
		};
	};
}
