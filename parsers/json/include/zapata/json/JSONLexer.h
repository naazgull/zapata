// Generated by Flexc++ V2.11.00 on Tue, 03 May 2022 15:51:20 +0100

#ifndef zptJSONLexer_H_INCLUDED_
#define zptJSONLexer_H_INCLUDED_

// $insert baseclass_h
#include <zapata/json/JSONLexerbase.h>

// $insert namespace-open
namespace zpt {

// $insert classHead
class JSONLexer : public JSONLexerBase {
  public:
    explicit JSONLexer(std::istream& in = std::cin,
                       std::ostream& out = std::cout,
                       bool keepCwd = true);

    JSONLexer(std::string const& infile, std::string const& outfile, bool keepCwd = true);

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
inline JSONLexer::JSONLexer(std::istream& in, std::ostream& out, bool keepCwd)
  : JSONLexerBase(in, out, keepCwd) {}

inline JSONLexer::JSONLexer(std::string const& infile, std::string const& outfile, bool keepCwd)
  : JSONLexerBase(infile, outfile, keepCwd) {}

// $insert inlineLexFunction
inline int
JSONLexer::lex() {
    return lex_();
}

inline void
JSONLexer::preCode() {
    // optionally replace by your own code
}

inline void
JSONLexer::postCode([[maybe_unused]] PostEnum_ type) {
    // optionally replace by your own code
}

inline void
JSONLexer::print() {
    print_();
}

// $insert namespace-close
}

#endif // JSONLexer_H_INCLUDED_
