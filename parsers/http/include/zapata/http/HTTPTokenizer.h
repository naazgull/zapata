// Generated by Bisonc++ V6.04.03 on Sun, 31 Jul 2022 15:42:50 +0100

#ifndef zptHTTPTokenizer_h_included
#define zptHTTPTokenizer_h_included

// $insert baseclass
#include <zapata/http/HTTPTokenizerbase.h>
// $insert scanner.h
#include <zapata/http/HTTPLexer.h>

// $insert namespace-open
namespace zpt {

// $insert undefparser
#undef HTTPTokenizer
// CAVEAT: between the baseclass-include directive and the
// #undef directive in the previous line references to HTTPTokenizer
// are read as HTTPTokenizerBase.
// If you need to include additional headers in this file
// you should do so after these comment-lines.

class HTTPTokenizer : public HTTPTokenizerBase {
    // $insert scannerobject
    HTTPScanner d_scanner;

  public:
    HTTPTokenizer() = default;
    int parse();

  private:
    void error(); // called on (syntax) errors
    int lex();    // returns the next token from the
                  // lexical scanner.
    void print(); // use, e.g., d_token, d_loc
    void exceptionHandler(std::exception const& exc);

    // support functions for parse():
    void executeAction_(int ruleNr);
    void errorRecovery_();
    void nextCycle_();
    void nextToken_();
    void print_();
};

// $insert namespace-close
} // namespace zpt

#endif
