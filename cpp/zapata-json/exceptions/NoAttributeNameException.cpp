#include "NoAttributeNameException.h"

zapata::NoAttributeNameException::NoAttributeNameException(string _in) : __what(_in){
}

zapata::NoAttributeNameException::~NoAttributeNameException() throw() {
}

const char* zapata::NoAttributeNameException::what() {
	return this->__what.data();
}
