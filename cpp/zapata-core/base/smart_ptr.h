#pragma once

#include <string>
#include <base/str_map.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class smart_counter {
		private:

		public:
			size_t __pointed;
			smart_counter();
			~smart_counter();

			void add();
			size_t release();

			inline friend ostream& operator<<(ostream& os, smart_counter& f) {
				os << "smart_counter<" << f.__pointed << ">";
				return os;
			}
	};


	class smart_ref_table : public str_map<smart_counter*> {
		public:
			smart_ref_table();
			~smart_ref_table();

			smart_counter* add(void* ptr);
			size_t release(void* ptr);
			void remove(void* ptr);
	};

	extern smart_ref_table* __memory;

	template<typename T>
	class smart_ptr {
		private:
			T* __target;

		public:
			typedef T element_type;

			explicit smart_ptr(T* ptr = NULL) throw () {
				this->__target = NULL;
				this->set(ptr);
			}

			explicit smart_ptr(T& ptr) throw () {
				this->__target = NULL;
				this->set(&ptr);
			}

			smart_ptr(smart_ptr& rhs) throw () {
				this->__target = NULL;
				this->set(rhs.__target);
			}

			template<typename Y>
			smart_ptr(smart_ptr<Y>& rhs) throw () {
				this->__target = NULL;
				this->set(rhs.__target);
			}

			smart_ptr& operator=(smart_ptr& rhs) throw () {
				this->set(rhs.__target);
				return *this;
			}

			template<typename Y>
			smart_ptr& operator=(smart_ptr<Y>& rhs) throw () {
				this->set(rhs.__target);
				return *this;
			}

			smart_ptr& operator=(T* rhs) throw () {
				this->set(rhs);
				return *this;
			}

			smart_ptr& operator=(T& rhs) throw () {
				this->set(&rhs);
				return *this;
			}

			~smart_ptr() throw () {
				if (this->__target != NULL && zapata::__memory->release(this->__target) == 0) {
					zapata::__memory->remove(this->__target);
					delete this->__target;
				}
			}

			T* get() const throw () {
				return this->__target;
			}
			T& operator*() const throw () {
				return *this->__target;
			}
			T* operator->() const throw () {
				return this->__target;
			}

			T* release() throw () {
				zapata::__memory->release(this->__target);

				T* tmp = this->__target;
				this->__target = NULL;
				return tmp;
			}

			void destroy() throw () {
				delete this->__target;
				this->__target = NULL;
			}

			void set(T* ptr = NULL) throw () {
				if (ptr == NULL) {
					ptr = new T();
				}
				if (this->__target != ptr) {
					if (this->__target != NULL && zapata::__memory->release(this->__target) == 0) {
						zapata::__memory->remove(this->__target);
						delete this->__target;
					}
					this->__target = ptr;
					zapata::__memory->add(ptr);
				}
			}

			friend ostream& operator<<(ostream& _out, smart_ptr<T>& _in) {
				_out << (string) *(_in.__target);
				return _out;
			}
			smart_ptr<T>& operator<<(const char* _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(string _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(bool _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(int _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(long _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(long long _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(double _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(ObjectOp _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(T& _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator<<(smart_ptr<T>& _in) {
				(*this->__target) << _in;
				return *this;
			}
			template<typename Y>
			smart_ptr<T>& operator<<(smart_ptr<Y>& _in) {
				(*this->__target) << _in;
				return *this;
			}
			template<typename Y>
			smart_ptr<T>& operator<<(Y& _in) {
				(*this->__target) << _in;
				return *this;
			}
			smart_ptr<T>& operator>>(string _in) {
				(*this->__target) >> _in;
				return *this;
			}
			smart_ptr<T>& operator>>(const char* _in) {
				(*this->__target) >> _in;
				return *this;
			}
			smart_ptr<T>& operator>>(long long _in) {
				(*this->__target) >> _in;
				return *this;
			}
			smart_ptr<T>& operator>>(ObjectOp _in) {
				(*this->__target) >> _in;
				return *this;
			}
			T& operator[](size_t _idx) {
				return (T&) this->__target->get(_idx);
			}
			T& operator[](const char* _idx){
				return (T&) this->__target->get(_idx);
			}

	};

}

