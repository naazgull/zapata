#include "ParserEOF.h"

zapata::ParserEOF::ParserEOF(string _in) : __what(_in){
}

zapata::ParserEOF::~ParserEOF() throw() {
}

const char* zapata::ParserEOF::what() {
	return this->__what.data();
}
