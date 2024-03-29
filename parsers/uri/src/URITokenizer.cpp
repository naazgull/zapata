// Generated by Bisonc++ V6.04.04 on Sun, 11 Dec 2022 19:21:59 +0000

// base/comment

// $insert class.ih
#include <zapata/uri/URITokenizerimpl.h>
#include <zapata/json/json.h>

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
inline bool operator&(StateType lhs, StateType rhs) { return (static_cast<int>(lhs) & rhs) != 0; }
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
    { 267, 4 }, // 1: exp ->  scheme object params anchor
    { 267, 3 }, // 2: exp ->  object params anchor
    { 268, 3 }, // 3: scheme (STRING) ->  STRING #0001 DOUBLE_DOT
    { 272, 0 }, // 4: #0001 ->  <empty>
    { 269, 2 }, // 5: object ->  server path
    { 269, 1 }, // 6: object ->  path
    { 273, 6 }, // 7: server (SLASH) ->  SLASH SLASH user STRING #0002 port
    { 277, 0 }, // 8: #0002 ->  <empty>
    { 273, 5 }, // 9: server (SLASH) ->  SLASH SLASH STRING #0003 port
    { 278, 0 }, // 10: #0003 ->  <empty>
    { 275, 3 }, // 11: user (STRING) ->  STRING #0004 AT
    { 279, 0 }, // 12: #0004 ->  <empty>
    { 276, 0 }, // 13: port ->  <empty>
    { 276, 2 }, // 14: port (DOUBLE_DOT) ->  DOUBLE_DOT STRING
    { 274, 0 }, // 15: path ->  <empty>
    { 274, 1 }, // 16: path (SLASH) ->  SLASH
    { 274, 4 }, // 17: path (SLASH) ->  SLASH STRING #0005 path
    { 280, 0 }, // 18: #0005 ->  <empty>
    { 274, 3 }, // 19: path (DOT) ->  DOT #0006 path
    { 281, 0 }, // 20: #0006 ->  <empty>
    { 274, 3 }, // 21: path (DOT_DOT) ->  DOT_DOT #0007 path
    { 282, 0 }, // 22: #0007 ->  <empty>
    { 270, 0 }, // 23: params ->  <empty>
    { 270, 2 }, // 24: params (QMARK) ->  QMARK paramslist
    { 283, 4 }, // 25: paramslist (STRING) ->  STRING #0008 EQ paramvalue
    { 284, 0 }, // 26: #0008 ->  <empty>
    { 283, 6 }, // 27: paramslist (E) ->  paramslist E STRING #0009 EQ paramvalue
    { 286, 0 }, // 28: #0009 ->  <empty>
    { 285, 0 }, // 29: paramvalue ->  <empty>
    { 285, 1 }, // 30: paramvalue (STRING) ->  STRING
    { 271, 0 }, // 31: anchor ->  <empty>
    { 271, 2 }, // 32: anchor (CARDINAL) ->  CARDINAL STRING
    { 287, 1 }, // 33: exp_$ ->  exp
};

// State info and SR_ transitions for each state.

SR_ const s_0[] = {
    { { REQ_DEF }, { 10 } }, { { 267 }, { 1 } }, // exp
    { { 268 }, { 2 } },                          // scheme
    { { 269 }, { 3 } },                          // object
    { { 257 }, { 4 } },                          // STRING
    { { 273 }, { 5 } },                          // server
    { { 274 }, { 6 } },                          // path
    { { 259 }, { 7 } },                          // SLASH
    { { 265 }, { 8 } },                          // DOT
    { { 266 }, { 9 } },                          // DOT_DOT
    { { 0 }, { -15 } },
};

SR_ const s_1[] = {
    { { REQ_TOKEN }, { 2 } },
    { { EOF_ }, { ACCEPT_ } },
    { { 0 }, { 0 } },
};

SR_ const s_2[] = {
    { { REQ_DEF }, { 7 } }, { { 269 }, { 10 } }, // object
    { { 273 }, { 5 } },                          // server
    { { 274 }, { 6 } },                          // path
    { { 259 }, { 7 } },                          // SLASH
    { { 265 }, { 8 } },                          // DOT
    { { 266 }, { 9 } },                          // DOT_DOT
    { { 0 }, { -15 } },
};

SR_ const s_3[] = {
    { { REQ_DEF }, { 3 } },
    { { 270 }, { 11 } }, // params
    { { 261 }, { 12 } }, // QMARK
    { { 0 }, { -23 } },
};

SR_ const s_4[] = {
    { { DEF_RED }, { 2 } },
    { { 272 }, { 13 } }, // #0001
    { { 0 }, { -4 } },
};

SR_ const s_5[] = {
    { { REQ_DEF }, { 5 } }, { { 274 }, { 14 } }, // path
    { { 259 }, { 15 } },                         // SLASH
    { { 265 }, { 8 } },                          // DOT
    { { 266 }, { 9 } },                          // DOT_DOT
    { { 0 }, { -15 } },
};

SR_ const s_6[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -6 } },
};

SR_ const s_7[] = {
    { { REQ_DEF }, { 3 } },
    { { 259 }, { 16 } }, // SLASH
    { { 257 }, { 17 } }, // STRING
    { { 0 }, { -16 } },
};

SR_ const s_8[] = {
    { { DEF_RED }, { 2 } },
    { { 281 }, { 18 } }, // #0006
    { { 0 }, { -20 } },
};

SR_ const s_9[] = {
    { { DEF_RED }, { 2 } },
    { { 282 }, { 19 } }, // #0007
    { { 0 }, { -22 } },
};

SR_ const s_10[] = {
    { { REQ_DEF }, { 3 } },
    { { 270 }, { 20 } }, // params
    { { 261 }, { 12 } }, // QMARK
    { { 0 }, { -23 } },
};

SR_ const s_11[] = {
    { { REQ_DEF }, { 3 } },
    { { 271 }, { 21 } }, // anchor
    { { 264 }, { 22 } }, // CARDINAL
    { { 0 }, { -31 } },
};

SR_ const s_12[] = {
    { { REQ_TOKEN }, { 3 } },
    { { 283 }, { 23 } }, // paramslist
    { { 257 }, { 24 } }, // STRING
    { { 0 }, { 0 } },
};

SR_ const s_13[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 258 }, { 25 } }, // DOUBLE_DOT
    { { 0 }, { 0 } },
};

SR_ const s_14[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -5 } },
};

SR_ const s_15[] = {
    { { REQ_DEF }, { 2 } },
    { { 257 }, { 17 } }, // STRING
    { { 0 }, { -16 } },
};

SR_ const s_16[] = {
    { { REQ_TOKEN }, { 3 } },
    { { 275 }, { 26 } }, // user
    { { 257 }, { 27 } }, // STRING
    { { 0 }, { 0 } },
};

SR_ const s_17[] = {
    { { DEF_RED }, { 2 } },
    { { 280 }, { 28 } }, // #0005
    { { 0 }, { -18 } },
};

SR_ const s_18[] = {
    { { REQ_DEF }, { 5 } }, { { 274 }, { 29 } }, // path
    { { 259 }, { 15 } },                         // SLASH
    { { 265 }, { 8 } },                          // DOT
    { { 266 }, { 9 } },                          // DOT_DOT
    { { 0 }, { -15 } },
};

SR_ const s_19[] = {
    { { REQ_DEF }, { 5 } }, { { 274 }, { 30 } }, // path
    { { 259 }, { 15 } },                         // SLASH
    { { 265 }, { 8 } },                          // DOT
    { { 266 }, { 9 } },                          // DOT_DOT
    { { 0 }, { -15 } },
};

SR_ const s_20[] = {
    { { REQ_DEF }, { 3 } },
    { { 271 }, { 31 } }, // anchor
    { { 264 }, { 22 } }, // CARDINAL
    { { 0 }, { -31 } },
};

SR_ const s_21[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -2 } },
};

SR_ const s_22[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 257 }, { 32 } }, // STRING
    { { 0 }, { 0 } },
};

SR_ const s_23[] = {
    { { REQ_DEF }, { 2 } },
    { { 263 }, { 33 } }, // E
    { { 0 }, { -24 } },
};

SR_ const s_24[] = {
    { { DEF_RED }, { 2 } },
    { { 284 }, { 34 } }, // #0008
    { { 0 }, { -26 } },
};

SR_ const s_25[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -3 } },
};

SR_ const s_26[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 257 }, { 35 } }, // STRING
    { { 0 }, { 0 } },
};

SR_ const s_27[] = {
    { { REQ_DEF }, { 4 } }, { { 278 }, { 36 } }, // #0003
    { { 279 }, { 37 } },                         // #0004
    { { 260 }, { -12 } },                        // AT
    { { 0 }, { -10 } },
};

SR_ const s_28[] = {
    { { REQ_DEF }, { 5 } }, { { 274 }, { 38 } }, // path
    { { 259 }, { 15 } },                         // SLASH
    { { 265 }, { 8 } },                          // DOT
    { { 266 }, { 9 } },                          // DOT_DOT
    { { 0 }, { -15 } },
};

SR_ const s_29[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -19 } },
};

SR_ const s_30[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -21 } },
};

SR_ const s_31[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -1 } },
};

SR_ const s_32[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -32 } },
};

SR_ const s_33[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 257 }, { 39 } }, // STRING
    { { 0 }, { 0 } },
};

SR_ const s_34[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 262 }, { 40 } }, // EQ
    { { 0 }, { 0 } },
};

SR_ const s_35[] = {
    { { DEF_RED }, { 2 } },
    { { 277 }, { 41 } }, // #0002
    { { 0 }, { -8 } },
};

SR_ const s_36[] = {
    { { REQ_DEF }, { 3 } },
    { { 276 }, { 42 } }, // port
    { { 258 }, { 43 } }, // DOUBLE_DOT
    { { 0 }, { -13 } },
};

SR_ const s_37[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 260 }, { 44 } }, // AT
    { { 0 }, { 0 } },
};

SR_ const s_38[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -17 } },
};

SR_ const s_39[] = {
    { { DEF_RED }, { 2 } },
    { { 286 }, { 45 } }, // #0009
    { { 0 }, { -28 } },
};

SR_ const s_40[] = {
    { { REQ_DEF }, { 3 } },
    { { 285 }, { 46 } }, // paramvalue
    { { 257 }, { 47 } }, // STRING
    { { 0 }, { -29 } },
};

SR_ const s_41[] = {
    { { REQ_DEF }, { 3 } },
    { { 276 }, { 48 } }, // port
    { { 258 }, { 43 } }, // DOUBLE_DOT
    { { 0 }, { -13 } },
};

SR_ const s_42[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -9 } },
};

SR_ const s_43[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 257 }, { 49 } }, // STRING
    { { 0 }, { 0 } },
};

SR_ const s_44[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -11 } },
};

SR_ const s_45[] = {
    { { REQ_TOKEN }, { 2 } },
    { { 262 }, { 50 } }, // EQ
    { { 0 }, { 0 } },
};

SR_ const s_46[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -25 } },
};

SR_ const s_47[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -30 } },
};

SR_ const s_48[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -7 } },
};

SR_ const s_49[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -14 } },
};

SR_ const s_50[] = {
    { { REQ_DEF }, { 3 } },
    { { 285 }, { 51 } }, // paramvalue
    { { 257 }, { 47 } }, // STRING
    { { 0 }, { -29 } },
};

SR_ const s_51[] = {
    { { DEF_RED }, { 1 } },
    { { 0 }, { -27 } },
};

// State array:
SR_ const* s_state[] = {
    s_0,  s_1,  s_2,  s_3,  s_4,  s_5,  s_6,  s_7,  s_8,  s_9,  s_10, s_11, s_12,
    s_13, s_14, s_15, s_16, s_17, s_18, s_19, s_20, s_21, s_22, s_23, s_24, s_25,
    s_26, s_27, s_28, s_29, s_30, s_31, s_32, s_33, s_34, s_35, s_36, s_37, s_38,
    s_39, s_40, s_41, s_42, s_43, s_44, s_45, s_46, s_47, s_48, s_49, s_50, s_51,
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
URITokenizerBase::URITokenizerBase()
  : d_token(Reserved_::UNDETERMINED_)
  ,
  // $insert baseclasscode
  d_requiredTokens_(0) {}

// base/clearin
void URITokenizerBase::clearin_() {
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

void URITokenizerBase::setDebug(bool mode) {
    d_actionCases_ = false;
    d_debug_ = mode;
}

void URITokenizerBase::setDebug(DebugMode_ mode) {
    d_actionCases_ = mode & ACTIONCASES;
    d_debug_ = mode & ON;
}

// base/lex
void URITokenizerBase::lex_(int token) {
    d_token = token;

    if (d_token <= 0) d_token = Reserved_::EOF_;

    d_terminalToken = true;
}

// base/lookup
int URITokenizerBase::lookup_() const {
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
void URITokenizerBase::pop_(size_t count) {
    if (d_stackIdx < static_cast<int>(count)) { ABORT(); }

    d_stackIdx -= count;
    d_state = d_stateStack[d_stackIdx].first;
    d_vsp = &d_stateStack[d_stackIdx];
}

// base/poptoken
void URITokenizerBase::popToken_() {
    d_token = d_next.first;
    d_val_ = std::move(d_next.second);

    d_next.first = Reserved_::UNDETERMINED_;
}

// base/push
void URITokenizerBase::push_(size_t state) {
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
void URITokenizerBase::pushToken_(int token) {
    d_next = TokenPair{ d_token, std::move(d_val_) };
    d_token = token;
}

// base/redotoken
void URITokenizerBase::redoToken_() {
    if (d_token != Reserved_::UNDETERMINED_) pushToken_(d_token);
}

// base/reduce
void URITokenizerBase::reduce_(int rule) {
    PI_ const& pi = s_productionInfo[rule];

    d_token = pi.d_nonTerm;
    pop_(pi.d_size);

    d_terminalToken = false;
}

// base/shift
void URITokenizerBase::shift_(int action) {
    push_(action);
    popToken_(); // token processed

    if (d_recovery and d_terminalToken) {
        d_recovery = false;
        d_acceptedTokens_ = 0;
    }
}

// base/startrecovery
void URITokenizerBase::startRecovery_() {
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
inline size_t URITokenizerBase::top_() const { return d_stateStack[d_stackIdx].first; }

// derived/errorrecovery
void URITokenizer::errorRecovery_() {
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
void URITokenizer::executeAction_(int production) try {
    if (token_() != Reserved_::UNDETERMINED_)
        pushToken_(token_()); // save an already available token
    switch (production) {
            // $insert actioncases

        case 1: {
            d_val_ = vs_(-3);
        } break;

        case 2: {
            d_val_ = vs_(-2);
        } break;

        case 3: {
            d_val_ = vs_(-2);
        } break;

        case 4: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                std::string _scheme{ d_scanner.matched() };
                if (!d_scanner.d_part_is_placeholder) {
                    auto _idx = _scheme.find("+");
                    if (_idx != std::string::npos) {
                        (*d_scanner)
                          << "scheme_options" << zpt::split(_scheme.substr(_idx + 1), "+", true);
                        _scheme.assign(_scheme.substr(0, _idx));
                    }
                }
                (*d_scanner) << "scheme" << _scheme;
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 5: {
            d_val_ = vs_(-1);
        } break;

        case 6: {
            d_val_ = vs_(0);
        } break;

        case 7: {
            d_val_ = vs_(-5);
        } break;

        case 8: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner) << "domain" << d_scanner.matched();
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 9: {
            d_val_ = vs_(-4);
        } break;

        case 10: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner) << "domain" << d_scanner.matched();
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 11: {
            d_val_ = vs_(-2);
        } break;

        case 12: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner) << "user" << zpt::json{ "name", d_scanner.matched() };
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 14: {
            int _port{ 0 };
            std::istringstream _iss;
            _iss.str(d_scanner.matched());
            _iss >> _port;
            if ((*d_scanner)->type() == zpt::JSObject) { (*d_scanner) << "port" << _port; }
            else { (*d_scanner) << _port; }
        } break;

        case 16: {
            if ((*d_scanner)->type() == zpt::JSObject) { (*d_scanner) << "is_relative" << false; }
        } break;

        case 17: {
            d_val_ = vs_(-3);
        } break;

        case 18: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                if (!(*d_scanner)("path")->ok()) {
                    (*d_scanner) << "path" << zpt::json::array();
                    (*d_scanner) << "raw_path"
                                 << "";
                    (*d_scanner) << "is_relative" << false;
                }
                (*d_scanner)["raw_path"]->string().append("/");
                (*d_scanner)["raw_path"]->string().append(d_scanner.matched());
                (*d_scanner)["path"] << zpt::url::r_decode(d_scanner.matched());
            }
            else { (*d_scanner) << zpt::url::r_decode(d_scanner.matched()); }
        } break;

        case 19: {
            d_val_ = vs_(-2);
        } break;

        case 20: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                if (!(*d_scanner)("path")->ok()) {
                    (*d_scanner) << "path" << zpt::json::array();
                    (*d_scanner) << "raw_path"
                                 << "";
                    (*d_scanner) << "is_relative" << true;
                }
                (*d_scanner)["raw_path"]->string().append("/.");
                (*d_scanner)["path"] << ".";
            }
            else { (*d_scanner) << "."; }
        } break;

        case 21: {
            d_val_ = vs_(-2);
        } break;

        case 22: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                if (!(*d_scanner)("path")->ok()) {
                    (*d_scanner) << "path" << zpt::json::array();
                    (*d_scanner) << "raw_path"
                                 << "";
                    (*d_scanner) << "is_relative" << true;
                }
                (*d_scanner)["raw_path"]->string().append("/..");
                (*d_scanner)["path"] << "..";
            }
            else { (*d_scanner) << ".."; }
        } break;

        case 24: {
            d_val_ = vs_(-1);
        } break;

        case 25: {
            d_val_ = vs_(-3);
        } break;

        case 26: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                if (!(*d_scanner)("params")->ok()) {
                    (*d_scanner) << "params" << zpt::json::object();
                }
                (*d_scanner) << "__aux" << d_scanner.matched();
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 27: {
            d_val_ = vs_(-5);
        } break;

        case 28: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner) << "__aux" << d_scanner.matched();
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 29: {
            auto __name = static_cast<std::string>((*d_scanner)["__aux"]);
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner)["params"] << __name << zpt::undefined;
            }
            else { (*d_scanner) << d_scanner.matched(); }
        } break;

        case 30: {
            auto __name = static_cast<std::string>((*d_scanner)["__aux"]);
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner)["params"] << __name << zpt::url::r_decode(d_scanner.matched());
            }
            else { (*d_scanner) << zpt::url::r_decode(d_scanner.matched()); }
        } break;

        case 32: {
            if ((*d_scanner)->type() == zpt::JSObject) {
                (*d_scanner) << "anchor" << zpt::url::r_decode(d_scanner.matched());
            }
            else { (*d_scanner) << zpt::url::r_decode(d_scanner.matched()); }
        } break;
    }
}
catch (std::exception const& exc) {
    exceptionHandler(exc);
}

// derived/nextcycle
void URITokenizer::nextCycle_() try {
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
void URITokenizer::nextToken_() {
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
void URITokenizer::print_() {
    // $insert print
}

// derived/parse
int URITokenizer::parse() try {
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
