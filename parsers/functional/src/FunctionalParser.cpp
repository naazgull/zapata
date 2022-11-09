#include <zapata/functional/FunctionalParser.h>

zpt::FunctionalParser::FunctionalParser(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}

zpt::FunctionalParser::~FunctionalParser() {}

void zpt::FunctionalParser::switchRoots(zpt::json& _root) { this->d_scanner.switchRoots(_root); }

void zpt::FunctionalParser::switchStreams(std::istream& _in, std::ostream& _out) {
    this->d_scanner.switchStreams(_in, _out);
}
