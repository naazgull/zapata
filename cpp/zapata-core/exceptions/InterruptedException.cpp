#include <exceptions/InterruptedException.h>

zapata::InterruptedException::InterruptedException(string _in) : __what(_in){
}

zapata::InterruptedException::~InterruptedException() throw() {
}

const char* zapata::InterruptedException::what() {
	return this->__what.data();
}
