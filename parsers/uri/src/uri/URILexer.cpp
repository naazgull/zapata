// Generated by Flexc++ V2.07.07 on Sat, 22 Aug 2020 09:11:19 -44391211

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

// $insert class_ih
#include <zapata/uri/URILexerimpl.h>

// $insert namespace-open
namespace zpt {

// s_ranges_: use (unsigned) characters as index to obtain
//           that character's range-number.
//           The range for EOF is defined in a constant in the
//           class header file
size_t const URILexerBase::s_ranges_[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  4,  4,  5,  6,  7,  8,  9,  9,  10, 11, 12, 13,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 16, 16, 17, 18, 19, 20, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 22, 23, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
};

// $insert startcondinfo
// s_dfa_ contains the rows of *all* DFAs ordered by start state.  The
// enum class StartCondition_is defined in the baseclass header.
// StartCondition_::INITIAL is always 0.  Each entry defines the row to
// transit to if the column's character range was sensed. Row numbers are
// relative to the used DFA, and d_dfaBase_ is set to the first row of
// the subset to use.  The row's final two values are respectively the
// rule that may be matched at this state, and the rule's FINAL flag. If
// the final value equals FINAL (= 1) then, if there's no continuation,
// the rule is matched. If the BOL flag (8) is also set (so FINAL + BOL (=
// 9) is set) then the rule only matches when d_atBOL is also true.
int const
  URILexerBase::s_dfa_[]
                      [29] = {
                          // INITIAL
                          { 1,  1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,  3,  4, 1,
                            -1, 1, 1, 1, 5, 1, 1, 6, 1, 1, 1, -1, -1, -1 }, // 0
                          { 1,  1, 1, -1, 1,  1, 1, 1,  1, 1, 1, 1,  -1, -1, 1,
                            -1, 1, 1, 1,  -1, 1, 1, -1, 1, 1, 1, -1, 0,  -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 6,  -1 }, // 2
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, -1 }, // 3
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2,  -1 }, // 4
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 5,  -1 }, // 5
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  -1 }, // 6
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  -1 }, // 7
                                                                                      // scheme
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1,  2, -1,
                            3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, // 0
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1 }, // 2
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7,  -1 }, // 3
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 9,  -1 }, // 4
                                                                                      // server_path
                          { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  2, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1 }, // 0
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, -1 }, // 2
                                                                                      // server
                          { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  2, 1,
                            3, 1, 1, 1, 1, 4, 1, 5, 1, 1, 1, -1, -1, -1 }, // 0
                          { 1,  1, 1, 1, 1, 1,  1, 1,  1, 1, 1, 1,  1,  -1, 1,
                            -1, 1, 1, 1, 1, -1, 1, -1, 1, 1, 1, -1, 13, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, -1 }, // 2
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1 }, // 3
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16, -1 }, // 4
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 17, -1 }, // 5
                                                                                      // path
                          { 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,  1,  3, 1,
                            1, 1, 1, 1, 4, 1, 1, 5, 1, 1, 1, -1, -1, -1 }, // 0
                          { 1, 1, 1, -1, 1,  1, 1, 1,  1, 1, 1, 1,  1,  -1, 1,
                            1, 1, 1, 1,  -1, 1, 1, -1, 1, 1, 1, -1, 22, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 21, -1 }, // 2
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 18, -1 }, // 3
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 20, -1 }, // 4
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 19, -1 }, // 5
                                                                                      // params
                          { 1, 1, 1, 2, 1, 3, 1, 4, 1, 1, 1, 1,  1,  1, 1,
                            1, 1, 5, 1, 1, 1, 1, 6, 1, 1, 1, -1, -1, -1 }, // 0
                          { 1, 1, 1,  -1, 1, -1, 1, -1, 1, 1, 1, 1,  1,  1, 1,
                            1, 1, -1, 1,  1, 1,  1, -1, 1, 1, 1, -1, 28, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 27, -1 }, // 2
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1 }, // 3
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, -1 }, // 4
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1 }, // 5
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1 }, // 6
                                                                                      // placeholder
                          { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, -1, -1, -1 }, // 0
                          { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1,  1,  1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, 29, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 30, -1 }, // 2
                                                                                      // function
                          { 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 1,  1,  1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1 }, // 0
                          { 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, -1, 1,  1,  1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1,  -1, 31, -1 }, // 1
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 33, -1 }, // 2
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 32, -1 }, // 3
                                                                                      // anchor
                          { 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1, 1,
                            1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1 }, // 0
                          { 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1, 1,
                            1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 34, -1 }, // 1
                      };

int const (*URILexerBase::s_dfaBase_[])[29] = {
    s_dfa_ + 0,  s_dfa_ + 8,  s_dfa_ + 13, s_dfa_ + 16, s_dfa_ + 22,
    s_dfa_ + 28, s_dfa_ + 35, s_dfa_ + 38, s_dfa_ + 42,
};

size_t URILexerBase::s_istreamNr = 0;

// $insert inputImplementation
URILexerBase::Input::Input()
  : d_in(0)
  , d_lineNr(1) {}

URILexerBase::Input::Input(std::istream* iStream, size_t lineNr)
  : d_in(iStream)
  , d_lineNr(lineNr) {}

size_t
URILexerBase::Input::get() {
    switch (size_t ch = next()) // get the next input char
    {
        case '\n':
            ++d_lineNr;
            [[fallthrough]];

        default:
            return ch;
    }
}

size_t
URILexerBase::Input::next() {
    size_t ch;

    if (d_deque.empty()) // deque empty: next char fm d_in
    {
        if (d_in == 0)
            return AT_EOF;
        ch = d_in->get();
        return *d_in ? ch : static_cast<size_t>(AT_EOF);
    }

    ch = d_deque.front();
    d_deque.pop_front();

    return ch;
}

void
URILexerBase::Input::reRead(size_t ch) {
    if (ch < 0x100) {
        if (ch == '\n')
            --d_lineNr;
        d_deque.push_front(ch);
    }
}

void
URILexerBase::Input::reRead(std::string const& str, size_t fm) {
    for (size_t idx = str.size(); idx-- > fm;)
        reRead(str[idx]);
}

URILexerBase::URILexerBase(std::istream& in, std::ostream& out)
  : d_filename("-")
  , d_out(new std::ostream(out.rdbuf()))
  ,
  // $insert interactiveInit
  d_in(0)
  , d_input(new std::istream(in.rdbuf()))
  , d_dfaBase_(s_dfa_) {}

void
URILexerBase::switchStream_(std::istream& in, size_t lineNr) {
    d_input.close();
    d_input = Input(new std::istream(in.rdbuf()), lineNr);
}

URILexerBase::URILexerBase(std::string const& infilename, std::string const& outfilename)
  : d_filename(infilename)
  , d_out(outfilename == "-" ? new std::ostream(std::cout.rdbuf())
                             : outfilename == "" ? new std::ostream(std::cerr.rdbuf())
                                                 : new std::ofstream(outfilename))
  , d_input(new std::ifstream(infilename))
  , d_dfaBase_(s_dfa_) {}

void
URILexerBase::switchStreams(std::istream& in, std::ostream& out) {
    switchStream_(in, 1);
    switchOstream(out);
}

void
URILexerBase::switchOstream(std::ostream& out) {
    *d_out << std::flush;
    d_out.reset(new std::ostream(out.rdbuf()));
}

// $insert debugFunctions
void
URILexerBase::setDebug(bool onOff) {}

bool
URILexerBase::debug() const {
    return false;
}

void
URILexerBase::redo(size_t nChars) {
    size_t from = nChars >= length() ? 0 : length() - nChars;
    d_input.reRead(d_matched, from);
    d_matched.resize(from);
}

void
URILexerBase::switchOstream(std::string const& outfilename) {
    *d_out << std::flush;
    d_out.reset(outfilename == "-" ? new std::ostream(std::cout.rdbuf())
                                   : outfilename == "" ? new std::ostream(std::cerr.rdbuf())
                                                       : new std::ofstream(outfilename));
}

void
URILexerBase::switchIstream(std::string const& infilename) {
    d_input.close();
    d_filename = infilename;
    d_input = Input(new std::ifstream(infilename));
    d_atBOL = true;
}

void
URILexerBase::switchStreams(std::string const& infilename, std::string const& outfilename) {
    switchOstream(outfilename);
    switchIstream(infilename);
}

void
URILexerBase::pushStream(std::istream& istr) {
    std::istream* streamPtr = new std::istream(istr.rdbuf());
    p_pushStream("(istream)", streamPtr);
}

void
URILexerBase::pushStream(std::string const& name) {
    std::istream* streamPtr = new std::ifstream(name);
    if (!*streamPtr) {
        delete streamPtr;
        throw std::runtime_error("Cannot read " + name);
    }
    p_pushStream(name, streamPtr);
}

void
URILexerBase::p_pushStream(std::string const& name, std::istream* streamPtr) {
    if (d_streamStack.size() == s_maxSizeofStreamStack_) {
        delete streamPtr;
        throw std::length_error("Max stream stack size exceeded");
    }

    d_streamStack.push_back(StreamStruct{ d_filename, d_input });
    d_filename = name;
    d_input = Input(streamPtr);
    d_atBOL = true;
}

bool
URILexerBase::popStream() {
    d_input.close();

    if (d_streamStack.empty())
        return false;

    StreamStruct& top = d_streamStack.back();

    d_input = top.pushedInput;
    d_filename = top.pushedName;
    d_streamStack.pop_back();

    return true;
}

// See the manual's section `Run-time operations' section for an explanation
// of this member.
URILexerBase::ActionType_
URILexerBase::actionType_(size_t range) {
    d_nextState = d_dfaBase_[d_state][range];

    if (d_nextState != -1) // transition is possible
        return ActionType_::CONTINUE;

    if (knownFinalState()) // FINAL state reached
        return ActionType_::MATCH;

    if (d_matched.size())
        return ActionType_::ECHO_FIRST; // no match, echo the 1st char

    return range != s_rangeOfEOF_ ? ActionType_::ECHO_CH : ActionType_::RETURN;
}

void
URILexerBase::accept(size_t nChars) // old name: less
{
    if (nChars < d_matched.size()) {
        d_input.reRead(d_matched, nChars);
        d_matched.resize(nChars);
    }
}

void
URILexerBase::setMatchedSize(size_t length) {
    d_input.reRead(d_matched, length); // reread the tail section
    d_matched.resize(length);          // return what's left
}

// At this point a rule has been matched.  The next character is not part of
// the matched rule and is sent back to the input.  The final match length
// is determined, the index of the matched rule is determined, and then
// d_atBOL is updated. Finally the rule's index is returned.
// The numbers behind the finalPtr assignments are explained in the
// manual's `Run-time operations' section.
size_t
URILexerBase::matched_(size_t ch) {
    d_input.reRead(ch);

    FinalData* finalPtr;

    if (not d_atBOL)             // not at BOL
        finalPtr = &d_final.std; // then use the std rule (3, 4)

    // at BOL
    else if (not available(d_final.std.rule)) // only a BOL rule avail.
        finalPtr = &d_final.bol;              // use the BOL rule (6)

    else if (not available(d_final.bol.rule)) // only a std rule is avail.
        finalPtr = &d_final.std;              // use the std rule (7)

    else if (               // Both are available (8)
      d_final.bol.length != // check lengths of matched texts
      d_final.std.length    // unequal lengths, use the rule
      )                     // having the longer match length
        finalPtr = d_final.bol.length > d_final.std.length ? &d_final.bol : &d_final.std;

    else // lengths are equal: use 1st rule
        finalPtr = d_final.bol.rule < d_final.std.rule ? &d_final.bol : &d_final.std;

    setMatchedSize(finalPtr->length);

    d_atBOL = d_matched.back() == '\n';

    return finalPtr->rule;
}

size_t
URILexerBase::getRange_(int ch) // using int to prevent casts
{
    return ch == AT_EOF ? as<size_t>(s_rangeOfEOF_) : s_ranges_[ch];
}

// At this point d_nextState contains the next state and continuation is
// possible. The just read char. is appended to d_match
void
URILexerBase::continue_(int ch) {
    d_state = d_nextState;

    if (ch != AT_EOF)
        d_matched += ch;
}

void
URILexerBase::echoCh_(size_t ch) {
    *d_out << as<char>(ch);
    d_atBOL = ch == '\n';
}

// At this point there is no continuation. The last character is
// pushed back into the input stream as well as all but the first char. in
// the buffer. The first char. in the buffer is echoed to stderr.
// If there isn't any 1st char yet then the current char doesn't fit any
// rules and that char is then echoed
void
URILexerBase::echoFirst_(size_t ch) {
    d_input.reRead(ch);
    d_input.reRead(d_matched, 1);
    echoCh_(d_matched[0]);
}

// Update the rules associated with the current state, do this separately
// for BOL and std rules.
// If a rule was set, update the rule index and the current d_matched
// length.
void
URILexerBase::updateFinals_() {
    size_t len = d_matched.size();

    int const* rf = d_dfaBase_[d_state] + s_finIdx_;

    if (rf[0] != -1) // update to the latest std rule
    {
        d_final.std = FinalData{ as<size_t>(rf[0]), len };
    }

    if (rf[1] != -1) // update to the latest bol rule
    {
        d_final.bol = FinalData{ as<size_t>(rf[1]), len };
    }
}

void
URILexerBase::reset_() {
    d_final = Final{ FinalData{ s_unavailable, 0 }, FinalData{ s_unavailable, 0 } };

    d_state = 0;
    d_return = true;

    if (!d_more)
        d_matched.clear();

    d_more = false;
}

int
URILexer::executeAction_(size_t ruleIdx) try {
    switch (ruleIdx) {
        // $insert actions
        case 0: {
            {
                begin(StartCondition_::scheme);
                d_part_is_placeholder = false;
                return zpt::uri::lex::STRING;
            }
        } break;
        case 1: {
            {
                d_path_helper.assign("{");
                d_intermediate_state = StartCondition_::scheme;
                begin(StartCondition_::placeholder);
            }
        } break;
        case 2: {
            {
                begin(StartCondition_::path);
                return zpt::uri::lex::SLASH;
            }
        } break;
        case 3: {
            {
                begin(StartCondition_::path);
                return zpt::uri::lex::DOT;
            }
        } break;
        case 4: {
            {
                begin(StartCondition_::path);
                return zpt::uri::lex::DOT_DOT;
            }
        } break;
        case 5: {
            {
                begin(StartCondition_::params);
                return zpt::uri::lex::QMARK;
            }
        } break;
        case 6: {
            {
                begin(StartCondition_::anchor);
                return zpt::uri::lex::CARDINAL;
            }
        } break;
        case 7: {
            {
                return zpt::uri::lex::DOUBLE_DOT;
            }
        } break;
        case 8: {
            {
                begin(StartCondition_::path);
                return zpt::uri::lex::DOT;
            }
        } break;
        case 9: {
            {
                begin(StartCondition_::path);
                return zpt::uri::lex::DOT_DOT;
            }
        } break;
        case 10: {
            {
                begin(StartCondition_::server_path);
                return zpt::uri::lex::SLASH;
            }
        } break;
        case 11: {
            {
                d_path_helper.assign(matched());
                begin(StartCondition_::path);
            }
        } break;
        case 12: {
            {
                begin(StartCondition_::server);
                return zpt::uri::lex::SLASH;
            }
        } break;
        case 13: {
            {
                d_server_part.assign(matched());
                d_part_is_placeholder = false;
                return zpt::uri::lex::STRING;
            }
        } break;
        case 14: {
            {
                setMatched(d_server_part);
                begin(StartCondition_::path);
                return zpt::uri::lex::SLASH;
            }
        } break;
        case 15: {
            {
                setMatched(d_server_part);
                return zpt::uri::lex::DOUBLE_DOT;
            }
        } break;
        case 16: {
            {
                setMatched(d_server_part);
                return zpt::uri::lex::AT;
            }
        } break;
        case 17: {
            {
                d_path_helper.assign("{");
                d_intermediate_state = StartCondition_::server;
                begin(StartCondition_::placeholder);
            }
        } break;
        case 18: {
            {
                d_path_helper.assign("");
                return zpt::uri::lex::SLASH;
            }
        } break;
        case 19: {
            {
                d_path_helper.assign("{");
                d_intermediate_state = StartCondition_::path;
                begin(StartCondition_::placeholder);
            }
        } break;
        case 20: {
            {
                begin(StartCondition_::params);
                return zpt::uri::lex::QMARK;
            }
        } break;
        case 21: {
            {
                begin(StartCondition_::anchor);
                return zpt::uri::lex::CARDINAL;
            }
        } break;
        case 22: {
            {
                std::string _matched{ matched() };
                _matched.insert(0, d_path_helper);
                d_path_helper.assign("");
                setMatched(_matched);
                d_part_is_placeholder = false;
                return zpt::uri::lex::STRING;
            }
        } break;
        case 23: {
            {
                return zpt::uri::lex::EQ;
            }
        } break;
        case 24: {
            {
                return zpt::uri::lex::E;
            }
        } break;
        case 25: {
            {
                d_path_helper.assign("{");
                d_intermediate_state = StartCondition_::params;
                begin(StartCondition_::placeholder);
            }
        } break;
        case 26: {
            {
                ++d_function_level;
                begin(StartCondition_::function);
            }
        } break;
        case 27: {
            {
                begin(StartCondition_::anchor);
                return zpt::uri::lex::CARDINAL;
            }
        } break;
        case 28: {
            {
                d_part_is_placeholder = false;
                return zpt::uri::lex::STRING;
            }
        } break;
        case 29: {
            {
                d_path_helper += matched();
            }
        } break;
        case 30: {
            {
                d_path_helper += matched();
                setMatched(d_path_helper);
                d_path_helper.assign("");
                begin(d_intermediate_state);
                d_part_is_placeholder = true;
                return zpt::uri::lex::STRING;
            }
        } break;
        case 31: {
            {
                if (matched().find("(") != std::string::npos) {
                    ++d_function_level;
                    d_function_helper = matched();
                }
                else {
                    return zpt::uri::lex::FUNCTION_PARAM;
                }
            }
        } break;
        case 32: {
            {
                if (d_function_level != 1) {
                    d_function_helper += ",";
                }
            }
        } break;
        case 33: {
            {
                --d_function_level;
                if (d_function_level == 0) {
                    begin(StartCondition_::params);
                }
                else {
                    d_function_helper += ")";
                    if (d_function_level == 1) {
                        setMatched(d_function_helper);
                        return zpt::uri::lex::FUNCTION_PARAM;
                    }
                }
            }
        } break;
        case 34: {
            {
                d_part_is_placeholder = false;
                return zpt::uri::lex::STRING;
            }
        } break;
    }
    noReturn_();
    return 0;
}
catch (Leave_ value) {
    return static_cast<int>(value);
}

int
URILexer::lex_() {
    reset_();
    preCode();

    while (true) {
        size_t ch = get_();           // fetch next char
        size_t range = getRange_(ch); // determine the range

        updateFinals_(); // update the state's Final info

        switch (actionType_(range)) // determine the action
        {
            case ActionType_::CONTINUE:
                continue_(ch);
                continue;

            case ActionType_::MATCH: {
                d_token_ = executeAction_(matched_(ch));
                if (return_()) {
                    print();
                    postCode(PostEnum_::RETURN);
                    return d_token_;
                }
                break;
            }

            case ActionType_::ECHO_FIRST:
                echoFirst_(ch);
                break;

            case ActionType_::ECHO_CH:
                echoCh_(ch);
                break;

            case ActionType_::RETURN:
                if (!popStream()) {
                    postCode(PostEnum_::END);
                    return 0;
                }
                postCode(PostEnum_::POP);
                continue;
        } // switch

        postCode(PostEnum_::WIP);

        reset_();
        preCode();
    } // while
}

void
URILexerBase::print_() const {}

// $insert namespace-close
}
