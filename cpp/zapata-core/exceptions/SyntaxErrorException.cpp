#include <exceptions/SyntaxErrorException.h>

zapata::SyntaxErrorException::SyntaxErrorException(string _in) : __what(_in){
}

zapata::SyntaxErrorException::~SyntaxErrorException() throw() {
}

const char* zapata::SyntaxErrorException::what() {
	return this->__what.data();
}
