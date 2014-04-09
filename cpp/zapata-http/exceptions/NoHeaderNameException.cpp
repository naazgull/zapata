#include "NoHeaderNameException.h"

zapata::NoHeaderNameException::NoHeaderNameException(string _in) : __what(_in){
}

zapata::NoHeaderNameException::~NoHeaderNameException() throw() {
}

const char* zapata::NoHeaderNameException::what() {
	return this->__what.data();
}
