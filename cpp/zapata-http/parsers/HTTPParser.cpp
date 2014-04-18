#include <parsers/HTTPParser.h>

zapata::HTTPParser::HTTPParser(std::istream &_in, std::ostream &_out, zapata::HTTPReq* _rootobj, zapata::HTTPRep* _rootarr) {
	this->d_scanner.switchStreams(_in, _out);
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

zapata::HTTPParser::~HTTPParser() {
}

void zapata::HTTPParser::switchRoots(HTTPReq* _rootobj, HTTPRep* _rootarr) {
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

void zapata::HTTPParser::switchStreams(std::istream &_in, std::ostream &_out) {
	this->d_scanner.switchStreams(_in, _out);
}

