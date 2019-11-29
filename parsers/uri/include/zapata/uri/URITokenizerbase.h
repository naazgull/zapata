// Generated by Bisonc++ V6.02.05 on Sun, 24 Nov 2019 21:03:50 +0000

// hdr/includes
#ifndef zptURITokenizerBase_h_included
#define zptURITokenizerBase_h_included

#include <exception>
#include <vector>
#include <iostream>
// $insert preincludes
#include "URIinc.h"

// hdr/baseclass

namespace // anonymous
{
    struct PI_;
}

// $insert namespace-open
namespace zpt
{


class URITokenizerBase
{
    public:
        enum DebugMode_
        {
            OFF           = 0,
            ON            = 1 << 0,
            ACTIONCASES   = 1 << 1
        };

// $insert tokens

    // Symbolic tokens:
    enum Tokens_
    {
        DOUBLE_DOT = 257,
        SLASH,
        AT,
        QMARK,
        EQ,
        E,
        STRING,
    };

// $insert STYPE
typedef int STYPE_;

    private:
                        // state  semval
        typedef std::pair<size_t, STYPE_> StatePair;
                       // token   semval
        typedef std::pair<int,    STYPE_> TokenPair;

        int d_stackIdx = -1;
        std::vector<StatePair> d_stateStack;
        StatePair  *d_vsp = 0;       // points to the topmost value stack
        size_t      d_state = 0;

        TokenPair   d_next;
        int         d_token;

        bool        d_terminalToken = false;
        bool        d_recovery = false;


    protected:
        enum Return_
        {
            PARSE_ACCEPT_ = 0,   // values used as parse()'s return values
            PARSE_ABORT_  = 1
        };
        enum ErrorRecovery_
        {
            UNEXPECTED_TOKEN_,
        };

        bool        d_actionCases_ = false;    // set by options/directives
        bool        d_debug_ = true;
        size_t      d_requiredTokens_;
        size_t      d_nErrors_;                // initialized by clearin()
        size_t      d_acceptedTokens_;
        STYPE_     d_val_;


        URITokenizerBase();

        void ABORT() const;
        void ACCEPT() const;
        void ERROR() const;

        STYPE_ &vs_(int idx);             // value stack element idx 
        int  lookup_() const;
        int  savedToken_() const;
        int  token_() const;
        size_t stackSize_() const;
        size_t state_() const;
        size_t top_() const;
        void clearin_();
        void errorVerbose_();
        void lex_(int token);
        void popToken_();
        void pop_(size_t count = 1);
        void pushToken_(int token);
        void push_(size_t nextState);
        void redoToken_();
        bool recovery_() const;
        void reduce_(int rule);
        void shift_(int state);
        void startRecovery_();

    public:
        void setDebug(bool mode);
        void setDebug(DebugMode_ mode);
}; 

// hdr/abort
inline void URITokenizerBase::ABORT() const
{
    throw PARSE_ABORT_;
}

// hdr/accept
inline void URITokenizerBase::ACCEPT() const
{
    throw PARSE_ACCEPT_;
}


// hdr/error
inline void URITokenizerBase::ERROR() const
{
    throw UNEXPECTED_TOKEN_;
}

// hdr/savedtoken
inline int URITokenizerBase::savedToken_() const
{
    return d_next.first;
}

// hdr/opbitand
inline URITokenizerBase::DebugMode_ operator&(URITokenizerBase::DebugMode_ lhs,
                                     URITokenizerBase::DebugMode_ rhs)
{
    return static_cast<URITokenizerBase::DebugMode_>(
            static_cast<int>(lhs) & rhs);
}

// hdr/opbitor
inline URITokenizerBase::DebugMode_ operator|(URITokenizerBase::DebugMode_ lhs, 
                                     URITokenizerBase::DebugMode_ rhs)
{
    return static_cast<URITokenizerBase::DebugMode_>(static_cast<int>(lhs) | rhs);
};

// hdr/recovery
inline bool URITokenizerBase::recovery_() const
{
    return d_recovery;
}

// hdr/stacksize
inline size_t URITokenizerBase::stackSize_() const
{
    return d_stackIdx + 1;
}

// hdr/state
inline size_t URITokenizerBase::state_() const
{
    return d_state;
}

// hdr/token
inline int URITokenizerBase::token_() const
{
    return d_token;
}

// hdr/vs
inline URITokenizerBase::STYPE_ &URITokenizerBase::vs_(int idx) 
{
    return (d_vsp + idx)->second;
}

// hdr/tail
// For convenience, when including ParserBase.h its symbols are available as
// symbols in the class Parser, too.
#define URITokenizer URITokenizerBase

// $insert namespace-close
}

#endif



