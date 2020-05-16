// Generated by Bisonc++ V6.03.00 on Sat, 16 May 2020 16:58:08 -44156758

#ifndef zptJSONTokenizer_h_included
#define zptJSONTokenizer_h_included

// $insert baseclass
#include <zapata/json/JSONTokenizerbase.h>
// $insert scanner.h
#include <zapata/json/JSONLexer.h>

// $insert namespace-open
namespace zpt {

#undef JSONTokenizer
// CAVEAT: between the baseclass-include directive and the
// #undef directive in the previous line references to JSONTokenizer
// are read as JSONTokenizerBase.
// If you need to include additional headers in this file
// you should do so after these comment-lines.

class JSONTokenizer : public JSONTokenizerBase {
    // $insert scannerobject
    JSONScanner d_scanner;

  public:
    JSONTokenizer() = default;
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
}

#endif
