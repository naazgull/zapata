// Generated by Flexc++ V2.07.07 on Sun, 17 May 2020 23:54:29 -44159854

#ifndef zptURILexer_H_INCLUDED_
#define zptURILexer_H_INCLUDED_

// $insert baseclass_h
#include "URILexerbase.h"

// $insert namespace-open
namespace zpt {

// $insert classHead
class URILexer : public URILexerBase {
  public:
    explicit URILexer(std::istream& in = std::cin, std::ostream& out = std::cout);

    URILexer(std::string const& infile, std::string const& outfile);

    // $insert lexFunctionDecl
    int lex();

  private:
    int lex_();
    int executeAction_(size_t ruleNr);

    void print();
    void preCode(); // re-implement this function for code that must
                    // be exec'ed before the patternmatching starts

    void postCode(PostEnum_ type);
    // re-implement this function for code that must
    // be exec'ed after the rules's actions.
};

// $insert scannerConstructors
inline URILexer::URILexer(std::istream& in, std::ostream& out)
  : URILexerBase(in, out) {}

inline URILexer::URILexer(std::string const& infile, std::string const& outfile)
  : URILexerBase(infile, outfile) {}

// $insert inlineLexFunction
inline int
URILexer::lex() {
    return lex_();
}

inline void
URILexer::preCode() {
    // optionally replace by your own code
}

inline void
URILexer::postCode([[maybe_unused]] PostEnum_ type) {
    // optionally replace by your own code
}

inline void
URILexer::print() {
    print_();
}

// $insert namespace-close
}

#endif // URILexer_H_INCLUDED_
