// Generated by Flexc++ V2.11.00 on Tue, 03 May 2022 16:22:23 +0100

#ifndef zptFunctionalLexer_H_INCLUDED_
#define zptFunctionalLexer_H_INCLUDED_

// $insert baseclass_h
#include "FunctionalLexerbase.h"

// $insert namespace-open
namespace zpt {

// $insert classHead
class FunctionalLexer : public FunctionalLexerBase {
  public:
    explicit FunctionalLexer(std::istream& in = std::cin, std::ostream& out = std::cout, bool keepCwd = true);

    FunctionalLexer(std::string const& infile, std::string const& outfile, bool keepCwd = true);

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
inline FunctionalLexer::FunctionalLexer(std::istream& in, std::ostream& out, bool keepCwd)
  : FunctionalLexerBase(in, out, keepCwd) {}

inline FunctionalLexer::FunctionalLexer(std::string const& infile, std::string const& outfile, bool keepCwd)
  : FunctionalLexerBase(infile, outfile, keepCwd) {}

// $insert inlineLexFunction
inline int FunctionalLexer::lex() { return lex_(); }

inline void FunctionalLexer::preCode() {
    // optionally replace by your own code
}

inline void FunctionalLexer::postCode([[maybe_unused]] PostEnum_ type) {
    // optionally replace by your own code
}

inline void FunctionalLexer::print() { print_(); }

// $insert namespace-close
} // namespace zpt

#endif // FunctionalLexer_H_INCLUDED_
