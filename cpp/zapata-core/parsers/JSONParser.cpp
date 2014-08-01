#include <parsers/JSONParser.h>

zapata::JSONParser::JSONParser(std::istream &_in, std::ostream &_out, zapata::JSONObj* _rootobj, zapata::JSONArr* _rootarr) {
	this->d_scanner.switchStreams(_in, _out);
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

zapata::JSONParser::~JSONParser() {
}

void zapata::JSONParser::switchRoots(JSONObj* _rootobj, JSONArr* _rootarr) {
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

void zapata::JSONParser::switchStreams(std::istream &_in, std::ostream &_out) {
	this->d_scanner.switchStreams(_in, _out);
}

