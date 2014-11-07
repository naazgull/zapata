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

#pragma once

#define JSON(z) ((zapata::JSONObj()) << z)
#define JSON_ARRAY(z) ((zapata::JSONArr()) << z)
#define JSON_NIL zapata::undefined;

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <zapata/base/assert.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class JSONElementT;
	class JSONObj;
	class JSONArr;

	class JSONPtr : public shared_ptr<JSONElementT> {
	public:
		JSONPtr();
		JSONPtr(JSONElementT* _target);
		virtual ~JSONPtr();

		JSONElementT& value();

		template <typename T>
		bool operator==(T _rhs);
		template <typename T>
		bool operator!=(T _rhs);
		template <typename T>
		JSONPtr& operator<<(T _in);
		template <typename T>
		JSONPtr& operator[](T _idx);

		operator string();
		operator bool();
		operator int();
		operator long();
		operator long long();
#ifdef __LP64__
		operator unsigned int();
#endif
		operator size_t();
		operator double();
		operator JSONObj();
		operator JSONArr();
		operator JSONObj&();
		operator JSONArr&();
	};

	typedef JSONPtr JSONElement;

	extern JSONPtr undefined;
	extern JSONPtr nilptr;


	class JSONObjT : public map< string, JSONPtr > {
	public: 
		JSONObjT();
		virtual ~JSONObjT();

		virtual void stringify(string& _out);
		virtual void stringify(ostream& _out);

		virtual void push(string _name);
		virtual void push(JSONElementT& _value);
		virtual void push(JSONElementT* _value);

		virtual void pop(int _idx);
		virtual void pop(size_t _idx);
		virtual void pop(const char* _idx);
		virtual void pop(string _idx);

		bool operator==(JSONObjT& _in);
		template <typename T>
		bool operator==(T _in) {
			return false;
		};
		bool operator!=(JSONObjT& _in);
		template <typename T>
		bool operator!=(T _in) {
			return true;
		};

		JSONPtr& operator[](int _idx);
		JSONPtr& operator[](size_t _idx);
		JSONPtr& operator[](const char* _idx);
		JSONPtr& operator[](string _idx);

		friend ostream& operator<<(ostream& _out, JSONObjT& _in) {
			_in.stringify(_out);
			return _out;
		};

	private:
		string __name;
	};

	class JSONArrT : public vector<JSONPtr > {
	public: 
		JSONArrT();
		virtual ~JSONArrT();

		virtual void stringify(string& _out);
		virtual void stringify(ostream& _out);

		virtual void push(JSONElementT& _value);
		virtual void push(JSONElementT* _value);

		virtual void pop(int _idx);
		virtual void pop(size_t _idx);
		virtual void pop(const char* _idx);
		virtual void pop(string _idx);

		bool operator==(JSONArrT& _in);
		template <typename T>
		bool operator==(T _in) {
			return false;
		};
		bool operator!=(JSONArrT& _in);
		template <typename T>
		bool operator!=(T _in) {
			return true;
		};

		JSONPtr& operator[](int _idx);
		JSONPtr& operator[](size_t _idx);
		JSONPtr& operator[](const char* _idx);
		JSONPtr& operator[](string _idx);

		friend ostream& operator<<(ostream& _out, JSONArrT& _in) {
			_in.stringify(_out);
			return _out;
		};

	};	

	class JSONObj : public shared_ptr<JSONObjT> {
	public:
		JSONObj();
		JSONObj(JSONObj& _rhs);
		JSONObj(JSONObjT* _target);
		virtual ~JSONObj();

		JSONObjT::iterator begin();
		JSONObjT::iterator end();

		template <typename T>
		bool operator==(T _rhs) {
			return *(this->get()) == _rhs;
		};
		template <typename T>
		bool operator!=(T _rhs) {
			return *(this->get()) != _rhs;
		};
		JSONObj& operator<<(string _in);
		JSONObj& operator<<(const char* _in);
		template <typename T>
		JSONObj& operator<<(T _in) {
			(* this)->push(new JSONElementT(_in));
			return * this;
		};
		template <typename T>
		JSONObj& operator>>(T _in) {
			(* this)->pop(_in);
			return * this;
		};
		template <typename T>
		JSONPtr& operator[](T _idx) {
			return (*(this->get()))[_idx];
		};

		friend ostream& operator<<(ostream& _out, JSONObj& _in) {
			_in->stringify(_out);
			return _out;
		};		
	};

	class JSONArr : public shared_ptr<JSONArrT> {
	public:
		JSONArr();
		JSONArr(JSONArr& _rhs);
		JSONArr(JSONArrT* _target);
		virtual ~JSONArr();

		JSONArrT::iterator begin();
		JSONArrT::iterator end();

		template <typename T>
		bool operator==(T _rhs) {
			return *(this->get()) == _rhs;
		};
		template <typename T>
		bool operator!=(T _rhs) {
			return *(this->get()) != _rhs;
		};
		template <typename T>
		JSONArr& operator<<(T _in) {
			(* this)->push(new JSONElementT(_in));
			return * this;
		};
		template <typename T>
		JSONArr& operator>>(T _in) {
			(* this)->pop(_in);
			return * this;
		};
		template <typename T>
		JSONPtr& operator[](T _idx) {
			return (*(this->get()))[_idx];
		};

		friend ostream& operator<<(ostream& _out, JSONArr& _in) {
			_in->stringify(_out);
			return _out;
		};		
	};

	typedef shared_ptr<string> JSONStr;

	typedef struct JSONStruct {
		JSONStruct() : __type(JSNil) { };
		~JSONStruct() { 
			switch(__type) {
				case zapata::JSObject : {
					__object.~JSONObj();
					break;
				} case zapata::JSArray : {
					__array.~JSONArr();
					break;
				} case zapata::JSString : {
					__string.~JSONStr();
					break;
				} default : {
					break;
				}
			} 
		};

		JSONStruct(JSONStruct const&) = delete;
		JSONStruct& operator=(JSONStruct const&) = delete;

		JSONStruct(JSONStruct&&) = delete;
		JSONStruct& operator=(JSONStruct&&) = delete;

		JSONType __type;
		union {
			JSONObj __object;
			JSONArr __array;
			JSONStr __string;
			long long __integer;
			double __double;
			bool __boolean;
			void* __nil;
			time_t __date;
		};
	} JSONUnion;

	class JSONElementT {
	public:
		JSONElementT();
		JSONElementT(JSONElementT& _element);
		JSONElementT(JSONObj& _value);
		JSONElementT(JSONArr& _value);
		JSONElementT(string _value);
		JSONElementT(const char* _value);
		JSONElementT(long long _value);
		JSONElementT(double _value);
		JSONElementT(bool _value);
		JSONElementT(time_t _value);
		JSONElementT(int _value);
		JSONElementT(size_t _value);
#ifdef __LP64__
		JSONElementT(unsigned int _value);
#endif
		virtual ~JSONElementT();

		virtual JSONType type();
		virtual void type(JSONType _in);
		virtual JSONUnion& value();
		virtual bool ok();
		virtual bool empty();
		virtual bool nil();

		virtual void assign(JSONElementT& _rhs);

		JSONElementT* parent();
		void parent(JSONElementT* _parent);

		virtual JSONObj& obj();
		virtual JSONArr& arr();
		string str();
		long long intr();
		double dbl();
		bool bln();
		time_t date();
		double number();

		JSONElementT& operator<<(const char* _in);
		JSONElementT& operator<<(string _in);
		JSONElementT& operator<<(JSONElementT*);
		template <typename T>
		JSONElementT& operator<<(T _in){
			assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
			switch(this->__target.__type) {
				case zapata::JSObject : {
					this->__target.__object->push(new JSONElementT(_in));
					break;
				} case zapata::JSArray : {
					this->__target.__array->push(new JSONElementT(_in));
					break;
				} default : {
					JSONElementT _e(_in);
					this->assign(_e);
					assertz(this->__target.__type >= 0, "the type must be a valid value", 0, 0);
					break;
				}
			}
			return * this;
		};
		template <typename T>
		JSONElementT& operator>>(T _in){
			switch(this->__target.__type) {
				case zapata::JSObject : {
					this->__target.__object >> _in;
					break;
				}
				case zapata::JSArray : {
					this->__target.__array >> _in;
					break;
				}
				default : {
					break;
				}
			}
			return * this;
		};
		template <typename T>
		JSONPtr& operator[](T _idx) {
			if (this->__target.__type == zapata::JSObject) {
				return this->__target.__object [_idx];
			}
			else if (this->__target.__type == zapata::JSArray) {
				return this->__target.__array [_idx];
			}
			return zapata::undefined;
		};
		bool operator==(JSONElementT& _in);
		template <typename T>
		bool operator==(T _in) {
			JSONElementT _rhs(_in);
			return (* this) == _rhs;
		};
		bool operator!=(JSONElementT& _in);
		template <typename T>
		bool operator!=(T _in) {
			JSONElementT _rhs(_in);
			return (* this) == _rhs;
		};

		friend ostream& operator<<(ostream& _out, JSONElementT _in) {
			_in.stringify(_out);
			return _out;
		};

		virtual void stringify(string& _out);
		virtual void stringify(ostream& _out);

	private:
		JSONUnion __target;
		JSONElementT* __parent;

		void init();
	};

}

template <typename T>
bool zapata::JSONPtr::operator==(T _rhs) {
	return *(this->get()) == _rhs;
};
template <typename T>
bool zapata::JSONPtr::operator!=(T _rhs){
	return *(this->get()) != _rhs;
};
template <typename T>
zapata::JSONPtr& zapata::JSONPtr::operator<<(T _in) {
	*(this->get()) << _in;
	return * this;
};
template <typename T>
zapata::JSONPtr& zapata::JSONPtr::operator[](T _idx) {
	return (*(this->get()))[_idx];
};
