// Generated by Bisonc++ V6.04.03 on Tue, 03 May 2022 16:22:15 +0100

// base/comment

// $insert class.ih
#include <zapata/functional/FunctionalTokenizerimpl.h>

// The FIRST element of SR arrays shown below uses `d_type', defining the
// state's type, and `d_lastIdx' containing the last element's index. If
// d_lastIdx contains the REQ_TOKEN bitflag (see below) then the state needs
// a token: if in this state d_token is Reserved_::UNDETERMINED_, nextToken() will be
// called

// The LAST element of SR arrays uses `d_token' containing the last retrieved
// token to speed up the (linear) seach.  Except for the first element of SR
// arrays, the field `d_action' is used to determine what to do next. If
// positive, it represents the next state (used with SHIFT); if zero, it
// indicates `ACCEPT', if negative, -d_action represents the number of the
// rule to reduce to.

// `lookup()' tries to find d_token in the current SR array. If it fails, and
// there is no default reduction UNEXPECTED_TOKEN_ is thrown, which is then
// caught by the error-recovery function.

// The error-recovery function will pop elements off the stack until a state
// having bit flag ERR_ITEM is found. This state has a transition on errTok_
// which is applied. In this errTok_ state, while the current token is not a
// proper continuation, new tokens are obtained by nextToken(). If such a
// token is found, error recovery is successful and the token is
// handled according to the error state's SR table and parsing continues.
// During error recovery semantic actions are ignored.

// A state flagged with the DEF_RED flag will perform a default
// reduction if no other continuations are available for the current token.

// The ACCEPT STATE never shows a default reduction: when it is reached the
// parser returns ACCEPT(). During the grammar
// analysis phase a default reduction may have been defined, but it is
// removed during the state-definition phase.

// So:
//      s_x[] =
//      {
//                  [_field_1_]         [_field_2_]
//
// First element:   {state-type,        idx of last element},
// Other elements:  {required token,    action to perform},
//                                      ( < 0: reduce,
//                                          0: ACCEPT,
//                                        > 0: next state)
//      }

// base/declarations

namespace // anonymous
{
char const author[] = "Frank B. Brokken (f.b.brokken@rug.nl)";

enum Reserved_ { UNDETERMINED_ = -2, EOF_ = -1, errTok_ = 256 };
enum StateType // modify statetype/data.cc when this enum changes
{
    NORMAL,
    ERR_ITEM,
    REQ_TOKEN,
    ERR_REQ,    // ERR_ITEM | REQ_TOKEN
    DEF_RED,    // state having default reduction
    ERR_DEF,    // ERR_ITEM | DEF_RED
    REQ_DEF,    // REQ_TOKEN | DEF_RED
    ERR_REQ_DEF // ERR_ITEM | REQ_TOKEN | DEF_RED
};
inline bool
operator&(StateType lhs, StateType rhs) {
    return (static_cast<int>(lhs) & rhs) != 0;
}
enum StateTransition {
    ACCEPT_ = 0, // `ACCEPT' TRANSITION
};

struct PI_ // Production Info
{
    size_t d_nonTerm; // identification number of this production's
                      // non-terminal
    size_t d_size;    // number of elements in this production
};

struct SR_ // Shift Reduce info, see its description above
{
    union {
        int _field_1_; // initializer, allowing initializations
                       // of the SR s_[] arrays
        StateType d_type;
        int d_token;
    };
    union {
        int _field_2_;

        int d_lastIdx; // if negative, the state uses SHIFT
        int d_action;  // may be negative (reduce),
                       // postive (shift), or 0 (accept)
    };
};

// $insert staticdata

enum // size to expand the state-stack with when
{    // full
    STACK_EXPANSION_ = 10
};

// Productions Info Records:
PI_ const s_productionInfo[] = {
    { 0, 0 },   // not used: reduction values are negative
    { 263, 2 }, // 1: exp ->  token params
    { 265, 0 }, // 2: params ->  <empty>
    { 265, 3 }, // 3: params (LPAREN) ->  LPAREN param_list RPAREN
    { 266, 0 }, // 4: param_list ->  <empty>
    { 266, 3 }, // 5: param_list ->  exp #0001 param_list
    { 267, 0 }, // 6: #0001 ->  <empty>
    { 266, 4 }, // 7: param_list (COMMA) ->  COMMA exp #0002 param_list
    { 268, 0 }, // 8: #0002 ->  <empty>
    { 264, 1 }, // 9: token (STRING) ->  STRING
    { 264, 1 }, // 10: token (NUMBER) ->  NUMBER
    { 264, 1 }, // 11: token (VARIABLE) ->  VARIABLE
    { 269, 1 }, // 12: exp_$ ->  exp
};

// State info and SR_ transitions for each state.

SR_ const s_0[] = {
    { { REQ_TOKEN }, { 6 } }, { { 263 }, { 1 } }, // exp
    { { 264 }, { 2 } },                           // token
    { { 257 }, { 3 } },                           // STRING
    { { 258 }, { 4 } },                           // NUMBER
    { { 259 }, { 5 } },                           // VARIABLE
    { { 0 }, { 0 } },
};

SR_ const s_1[] = {
    { { REQ_TOKEN }, { 2 } },
    { { EOF_ }, { ACCEPT_ } },
    { { 0 }, { 0 } },
};

SR_ const s_2[] = {
    { { REQ_DEF }, { 3 } },
    { { 265 }, { 6 } }, // params
    { { 260 }, { 7 } }, // LPAREN
    { { 0 }, { -2 } },
};

SR_ const s_3[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -9 } },
};

SR_ const s_4[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -10 } },
};

SR_ const s_5[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -11 } },
};

SR_ const s_6[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -1 } },
};

SR_ const s_7[] = {
    { { REQ_DEF }, { 8 } }, { { 266 }, { 8 } }, // param_list
    { { 263 }, { 9 } },                         // exp
    { { 262 }, { 10 } },                        // COMMA
    { { 264 }, { 2 } },                         // token
    { { 257 }, { 3 } },                         // STRING
    { { 258 }, { 4 } },                         // NUMBER
    { { 259 }, { 5 } },                         // VARIABLE
    { { 0 }, { -4 } },
};

SR_ const s_8[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 261 }, { 11 } }, // RPAREN
    { { 0 }, { 0 } },
};

SR_ const s_9[] = {
    { { DEF_RED }, { 2 } },
    { { 267 }, { 12 } }, // #0001
    { { 0 }, { -6 } },
};

SR_ const s_10[] = {
    { { REQ_TOKEN }, { 6 } }, { { 263 }, { 13 } }, // exp
    { { 264 }, { 2 } },                            // token
    { { 257 }, { 3 } },                            // STRING
    { { 258 }, { 4 } },                            // NUMBER
    { { 259 }, { 5 } },                            // VARIABLE
    { { 0 }, { 0 } },
};

SR_ const s_11[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -3 } },
};

SR_ const s_12[] = {
    { { REQ_DEF }, { 8 } }, { { 266 }, { 14 } }, // param_list
    { { 263 }, { 9 } },                          // exp
    { { 262 }, { 10 } },                         // COMMA
    { { 264 }, { 2 } },                          // token
    { { 257 }, { 3 } },                          // STRING
    { { 258 }, { 4 } },                          // NUMBER
    { { 259 }, { 5 } },                          // VARIABLE
    { { 0 }, { -4 } },
};

SR_ const s_13[] = {
    { { DEF_RED }, { 2 } },
    { { 268 }, { 15 } }, // #0002
    { { 0 }, { -8 } },
};

SR_ const s_14[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -5 } },
};

SR_ const s_15[] = {
    { { REQ_DEF }, { 8 } }, { { 266 }, { 16 } }, // param_list
    { { 263 }, { 9 } },                          // exp
    { { 262 }, { 10 } },                         // COMMA
    { { 264 }, { 2 } },                          // token
    { { 257 }, { 3 } },                          // STRING
    { { 258 }, { 4 } },                          // NUMBER
    { { 259 }, { 5 } },                          // VARIABLE
    { { 0 }, { -4 } },
};

SR_ const s_16[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -7 } },
};

// State array:
SR_ const* s_state[] = {
    s_0, s_1, s_2, s_3, s_4, s_5, s_6, s_7, s_8, s_9, s_10, s_11, s_12, s_13, s_14, s_15, s_16,
};

} // namespace

// $insert namespace-open
namespace zpt {

// If the parsing function call (i.e., parse()' needs arguments, then provide
// an overloaded function.  The code below doesn't rely on parameters, so no
// arguments are required.  Furthermore, parse uses a function try block to
// allow us to do ACCEPT and ABORT from anywhere, even from within members
// called by actions, simply throwing the appropriate exceptions.

// base/base1
FunctionalTokenizerBase::FunctionalTokenizerBase()
  : d_token(Reserved_::UNDETERMINED_)
  ,
  // $insert baseclasscode
  d_requiredTokens_(0) {}

// base/clearin
void
FunctionalTokenizerBase::clearin_() {
    d_nErrors_ = 0;
    d_stackIdx = -1;
    d_stateStack.clear();
    d_token = Reserved_::UNDETERMINED_;
    d_next = TokenPair{ Reserved_::UNDETERMINED_, STYPE_{} };
    d_recovery = false;
    d_acceptedTokens_ = d_requiredTokens_;
    d_val_ = STYPE_{};

    push_(0);
}

// base/debugfunctions

void
FunctionalTokenizerBase::setDebug(bool mode) {
    d_actionCases_ = false;
    d_debug_ = mode;
}

void
FunctionalTokenizerBase::setDebug(DebugMode_ mode) {
    d_actionCases_ = mode & ACTIONCASES;
    d_debug_ = mode & ON;
}

// base/lex
void
FunctionalTokenizerBase::lex_(int token) {
    d_token = token;

    if (d_token <= 0) d_token = Reserved_::EOF_;

    d_terminalToken = true;
}

// base/lookup
int
FunctionalTokenizerBase::lookup_() const {
    // if the final transition is negative, then we should reduce by the rule
    // given by its positive value.

    SR_ const* sr = s_state[d_state];
    SR_ const* last = sr + sr->d_lastIdx;

    for (; ++sr != last;) // visit all but the last SR entries
    {
        if (sr->d_token == d_token) return sr->d_action;
    }

    if (sr == last) // reached the last element
    {
        if (sr->d_action < 0) // default reduction
        {
            return sr->d_action;
        }

        // No default reduction, so token not found, so error.
        throw UNEXPECTED_TOKEN_;
    }

    // not at the last element: inspect the nature of the action
    // (< 0: reduce, 0: ACCEPT, > 0: shift)

    int action = sr->d_action;

    return action;
}

// base/pop
void
FunctionalTokenizerBase::pop_(size_t count) {
    if (d_stackIdx < static_cast<int>(count)) { ABORT(); }

    d_stackIdx -= count;
    d_state = d_stateStack[d_stackIdx].first;
    d_vsp = &d_stateStack[d_stackIdx];
}

// base/poptoken
void
FunctionalTokenizerBase::popToken_() {
    d_token = d_next.first;
    d_val_ = std::move(d_next.second);

    d_next.first = Reserved_::UNDETERMINED_;
}

// base/push
void
FunctionalTokenizerBase::push_(size_t state) {
    size_t currentSize = d_stateStack.size();
    if (stackSize_() == currentSize) {
        size_t newSize = currentSize + STACK_EXPANSION_;
        d_stateStack.resize(newSize);
    }

    ++d_stackIdx;
    d_stateStack[d_stackIdx] = StatePair{ d_state = state, std::move(d_val_) };

    d_vsp = &d_stateStack[d_stackIdx];

    if (d_stackIdx == 0) {}
    else {}
}

// base/pushtoken
void
FunctionalTokenizerBase::pushToken_(int token) {
    d_next = TokenPair{ d_token, std::move(d_val_) };
    d_token = token;
}

// base/redotoken
void
FunctionalTokenizerBase::redoToken_() {
    if (d_token != Reserved_::UNDETERMINED_) pushToken_(d_token);
}

// base/reduce
void
FunctionalTokenizerBase::reduce_(int rule) {
    PI_ const& pi = s_productionInfo[rule];

    d_token = pi.d_nonTerm;
    pop_(pi.d_size);

    d_terminalToken = false;
}

// base/shift
void
FunctionalTokenizerBase::shift_(int action) {
    push_(action);
    popToken_(); // token processed

    if (d_recovery and d_terminalToken) {
        d_recovery = false;
        d_acceptedTokens_ = 0;
    }
}

// base/startrecovery
void
FunctionalTokenizerBase::startRecovery_() {
    int lastToken = d_token; // give the unexpected token a
                             // chance to be processed
                             // again.

    pushToken_(Reserved_::errTok_); // specify errTok_ as next token
    push_(lookup_());               // push the error state

    d_token = lastToken; // reactivate the unexpected
                         // token (we're now in an
                         // ERROR state).

    d_recovery = true;
}

// base/top
inline size_t
FunctionalTokenizerBase::top_() const {
    return d_stateStack[d_stackIdx].first;
}

// derived/errorrecovery
void
FunctionalTokenizer::errorRecovery_() {
    // When an error has occurred, pop elements off the stack until the top
    // state has an error-item. If none is found, the default recovery
    // mode (which is to abort) is activated.
    //
    // If EOF is encountered without being appropriate for the current state,
    // then the error recovery will fall back to the default recovery mode.
    // (i.e., parsing terminates)

    if (d_acceptedTokens_ >= d_requiredTokens_) // only generate an error-
    {                                           // message if enough tokens
        ++d_nErrors_;                           // were accepted. Otherwise
        error();                                // simply skip input
    }

    // get the error state
    while (not(s_state[top_()][0].d_type & ERR_ITEM)) { pop_(); }

    // In the error state, looking up a token allows us to proceed.
    // Continuation may be require multiple reductions, but eventually a
    // terminal-token shift is used. See nextCycle_ for details.

    startRecovery_();
}

// derived/executeaction
void
FunctionalTokenizer::executeAction_(int production) try {
    if (token_() != Reserved_::UNDETERMINED_) pushToken_(token_()); // save an already available token
    switch (production) {
            // $insert actioncases

        case 1: {
            d_val_ = vs_(-1);
        } break;

        case 3: {
            d_val_ = vs_(-2);
        } break;

        case 5: {
            d_val_ = vs_(-2);
        } break;

        case 6: {
            d_scanner.add_param();
        } break;

        case 7: {
            d_val_ = vs_(-3);
        } break;

        case 8: {
            d_scanner.add_param();
        } break;

        case 9: {
            d_scanner.set_string();
        } break;

        case 10: {
            d_scanner.set_number();
        } break;

        case 11: {
            d_scanner.set_variable();
        } break;
    }
}
catch (std::exception const& exc) {
    exceptionHandler(exc);
}

// derived/nextcycle
void
FunctionalTokenizer::nextCycle_() try {
    if (s_state[state_()]->d_type & REQ_TOKEN) nextToken_(); // obtain next token

    int action = lookup_(); // lookup d_token in d_state

    if (action > 0) // SHIFT: push a new state
    {
        shift_(action);
        return;
    }

    if (action < 0) // REDUCE: execute and pop.
    {

        if (recovery_())
            redoToken_();
        else
            executeAction_(-action);
        // next token is the rule's LHS
        reduce_(-action);
        return;
    }

    if (recovery_())
        ABORT();
    else
        ACCEPT();
}
catch (ErrorRecovery_) {
    if (not recovery_())
        errorRecovery_();
    else {
        if (token_() == Reserved_::EOF_) ABORT();
        popToken_(); // skip the failing token
    }
}

// derived/nexttoken
void
FunctionalTokenizer::nextToken_() {
    // If d_token is Reserved_::UNDETERMINED_ then if savedToken_() is
    // Reserved_::UNDETERMINED_ another token is obtained from lex(). Then
    // savedToken_() is assigned to d_token.

    // no need for a token: got one already
    if (token_() != Reserved_::UNDETERMINED_) { return; }

    if (savedToken_() != Reserved_::UNDETERMINED_) {
        popToken_(); // consume pending token
    }
    else {
        ++d_acceptedTokens_; // accept another token (see
                             // errorRecover())
        lex_(lex());
        print_();
    }
    print();
}

// derived/print
void
FunctionalTokenizer::print_() {
    // $insert print
}

// derived/parse
int
FunctionalTokenizer::parse() try {
    // The parsing algorithm:
    // Initially, state 0 is pushed on the stack, and all relevant variables
    // are initialized by Base::clearin_.
    //
    // Then, in an eternal loop:
    //
    //  1. If a state is a REQ_TOKEN type, then the next token is obtained
    //     from nextToken().  This may very well be the currently available
    //     token. When retrieving a terminal token d_terminal is set to true.
    //
    //  2. lookup() is called, d_token is looked up in the current state's
    //     SR_ array.
    //
    //  4. Depending on the result of the lookup() function the next state is
    //     shifted on the parser's stack, a reduction by some rule is applied,
    //     or the parsing function returns ACCEPT(). When a reduction is
    //     called for, any action that may have been defined for that
    //     reduction is executed.
    //
    //  5. An error occurs if d_token is not found, and the state has no
    //     default reduction.

    clearin_(); // initialize, push(0)

    while (true) {
        // $insert prompt
        nextCycle_();
    }
}
catch (Return_ retValue) {
    return retValue or d_nErrors_;
}

// derived/tail

// $insert namespace-close
} // namespace zpt
