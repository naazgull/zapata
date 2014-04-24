#include "ClosedException.h"

zapata::ClosedException::ClosedException(string _in) : __what(_in){
}

zapata::ClosedException::~ClosedException() throw() {
}

const char* zapata::ClosedException::what() {
	return this->__what.data();
}
