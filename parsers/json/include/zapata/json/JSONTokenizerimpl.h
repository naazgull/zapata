// Generated by Bisonc++ V6.04.03 on Sun, 02 Oct 2022 20:24:30 +0100

// Include this file in the sources of the class JSONTokenizer.

// $insert class.h
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/json/JSONTokenizer.h>

// $insert namespace-open
namespace zpt {

inline void JSONTokenizer::error() {
    throw zpt::SyntaxErrorException(std::string("JSON: Syntax error in line ") +
                                    std::to_string(d_scanner.lineNr()));
}

// $insert lex
inline int JSONTokenizer::lex() { return d_scanner.lex(); }

inline void JSONTokenizer::print() {}

inline void JSONTokenizer::exceptionHandler(std::exception const& exc) {
    throw; // re-implement to handle exceptions thrown by actions
}

// $insert namespace-close
} // namespace zpt

// Add here includes that are only required for the compilation
// of JSONTokenizer's sources.

// $insert namespace-use
// UN-comment the next using-declaration if you want to use
// symbols from the namespace zpt without specifying zpt::
// using namespace zpt;

// UN-comment the next using-declaration if you want to use
// int JSONTokenizer's sources symbols from the namespace std without
// specifying std::

// using namespace std;
