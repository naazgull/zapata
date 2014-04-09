#include "CastException.h"

zapata::CastException::CastException(string _in) : __what(_in){
}

zapata::CastException::~CastException() throw() {
}

const char* zapata::CastException::what() {
	return this->__what.data();
}
